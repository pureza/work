#! /usr/bin/env ruby

require 'socket'
require 'logger'

class FaultClient

    def initialize(host, port, injector_device, script, logger)
        @socket = TCPSocket.new(host, port)
        @injector_device = injector_device
        @script = script
        @logger = logger
    end

    def start
        faults = nil
        while true
            message = @socket.gets.strip

            case message
            when /fault/
                faults = message.split[1..-1].join.split("|")
                @logger.info("received #{faults.length} faults")

                File.open(@injector_device, "w") do |dev|
                    dev.write("+" + faults.join("\n+"))
                end

                @logger.info("#{faults.length} faults injected")
                @socket.puts("#{faults.length} faults injected")
            when /execute/
                @logger.info("started workload")
                @socket.puts("started workload")

                # Run the workload and save its output to the log
                result = ""
                IO.popen("bash #{@script} 2>&1") do |pipe|
                    @logger.info("Output #{'#' * 40}")
                    while out = pipe.gets do
                        @logger.info(" > " + out)
                        result = out
                    end
                    @logger.info("#{'#' * 47}")
                end

                @logger.info("workload ended with result '#{result}'")
                @socket.puts(result)

                if !faults.nil?
                    File.open(@injector_device, "w") do |dev|
                        dev.write("-" + faults.join("\n+"))
                    end

                    @logger.info("#{faults.length} faults removed")
                    @socket.puts("#{faults.length} faults removed")
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

# Create the logger
logger = Logger.new("client.log")
logger.datetime_format = "%a %Y/%m/%d %H:%M:%S"
logger.info("client started #{'#' * 80}")

client = FaultClient.new(SERVER_IP, PORT, INJECTOR_DEVICE, SCRIPT, logger)
client.start
