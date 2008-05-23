-module(pc).
-compile(export_all).

start() ->
    % spawn the counter process
    Counter = spawn(fun counter/0),

    % spawn the buffer process
    Buffer = spawn(fun buffer/0),

    % spawn 10 consumers
    [spawn(fun() -> consumer(N, Buffer, 10) end) || N <- lists:seq(1, 10)],

    % spawn 10 producers
    [spawn(fun() -> producer(N, Buffer, Counter, 10) end) || N <- lists:seq(1, 10)].



%
% The counter generates increasing values, to be used by the producer
%
counter() ->
    counter(0).

counter(Value) ->
    receive 
	% Receive an increment counter order issued by a producer.
	{From, increment} ->
	    % Send the next value to the producer
	    From ! {self(), {value, Value + 1}},
	    counter(Value + 1)
    end.
	
%
% The buffer maintains a list of produced but not consumed values, 
% and a list of waiting consumers.
%
% FIXME: Note that the list of produced items is unbounded... is there a way
%        to avoid this (without throwing the produced items in surplus away)?
%
buffer() ->
    buffer([], []).

buffer(Produced, OnHold) ->
    receive
	% Receive a new value from a producer
	{_From, {produce, Value}} ->
	    % If there are consumers on hold, lets service the first one
	    % right away! Otherwise, append the new value to the list of
	    % produced values.
	    if length(OnHold) > 0 ->
		    [First | Rest] = OnHold,
		    First ! {self(), Value},
		    buffer(Produced, Rest);
	       true ->
		    buffer(Produced ++ [Value], OnHold)
	    end;
	% Receive a consume request from a consumer
	{From, consume} ->
	    % If there are values ready to be used, let's consume them!
	    % Otherwise, append the consumer to the list of waiting consumers
	    if length(Produced) > 0 ->
		    [First | Rest] = Produced,
		    From ! {self(), First},
		    buffer(Rest, OnHold);
	       true ->
		    buffer(Produced, OnHold ++ [From])
	    end	      
    end.

%
% The producer gets a new value from the counter and sends it
% to the buffer, until Left reaches 0.
%
producer(_Id, _Buffer, _Counter, 0) ->
    void;
producer(Id, Buffer, Counter, Left) ->
    Counter ! {self(), increment},
    receive
	% Receive Counter value - respond to the original consumer
	{Counter, {value, Value}} ->
	    io:format("Produtor ~w produziu ~w~n", [Id, Value]),
	    Buffer ! {self(), {produce, Value}},
	    producer(Id, Buffer, Counter, Left - 1)
    end.


%
% The consumer simply requests new values and prints them.
% It stops after Left times.
%
consumer(_Id, _Buffer, 0) ->
    void;
consumer(Id, Buffer, Left) ->
    Buffer ! {self(), consume},
    receive
	{Buffer, Value} ->
	    io:format("Consumindo ~w~n", [Value]),
            consumer(Id, Buffer, Left - 1)
    end.
    
