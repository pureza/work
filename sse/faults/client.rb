#! /usr/bin/env ruby

require 'socket'

class FaultClient
  
  def initialize(host, port, injector_device, script)
    @socket = TCPSocket.new(host, port)
    @injector_device = injector_device
    @script = script
  end

  def start
    faults = nil
    while true
      message = @socket.gets.strip

      case message
      when /FAULT/
        faults = message.split[1..-1].join.split("|")
        puts "applying #{faults.length} faults..."
        
        File.open(@injector_device, "w") do |dev|
          dev.write("+" + faults.join("\n+"))
        end
      when /EXECUTE/
        puts "executing..."
        result = `bash #{@script}`
        @socket.puts(result)

        if !faults.nil?
          puts "removing #{faults.length} faults..."
          File.open(@injector_device, "w") do |dev|
            dev.write("-" + faults.join("\n+"))
          end
        end
        faults = nil
      end
    end
  end

end

if ARGV.length != 4
  puts "Usage: client <server ip> <port> <injector device> <workload script>"
  exit
end

SERVER_IP = ARGV[0]
PORT = ARGV[1]
INJECTOR_DEVICE = ARGV[2]
SCRIPT = ARGV[3]

client = FaultClient.new(SERVER_IP, PORT, INJECTOR_DEVICE, SCRIPT)
client.start
