#! /usr/bin/ruby -w

require 'socket'
require 'timeout'
require 'thread'
require 'logger'
require 'yaml'
require 'vm.rb'
require 'barrier.rb'

# This is the correct result that all virtual machines should return
CORRECT_RESULT = "12573bcc96fb240ffd5f72e939b7528d"

# Number of seconds the server waits for the VM's answer
TIMEOUT = 400

# Number of faults to inject per round
FAULTS_PER_ROUND = 10

class FaultServer
    
    def initialize(port, vms, faults, logger)
        # The faults
        @faults = faults

        # Number of faults injected
        @injected_faults = 0

        # The virtual machines
        @vms = vms

        # This mutex synchronizes access to the VM's hashtable
        @vms_mutex = Mutex.new

        # VM's wait for each other before running their workload
        @barrier = Barrier.new(@vms.length)

        # The server's socket
        @socket = TCPServer.new("", port)
        @socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_REUSEADDR, 1)

        # Logger
        @logger = logger
    end

    def start
        while true
            # Accept a new connection...
            client_socket = @socket.accept
            
            # ... and handle it inside a new thread
            Thread.new(client_socket) do |client|
                begin
                    ip = client.peeraddr[3]
                    @logger.info("#{ip} connected")
                    handle_client(client)
                rescue Exception => ex
                    @logger.error("#{ip}: #{ex}\n\t from: " + ex.backtrace.join("\n\t from: "))
                ensure
                    begin 
                        client.close 
                    rescue 
                    end
                end
            end
        end
    end

    private

    def handle_client(socket)
        result = nil
        ip = socket.peeraddr[3]
        is_faulty = false
        step = "Initializing..."

        # Change the VM status to connected
        @vms_mutex.synchronize {
            throw "Unknown VM connection from '#{ip}'" unless @vms.has_key?(ip)

            if @vms[ip].status == :connected
                @vms[ip].reboots += 1
                @logger.error("#{ip} rebooted by itself #{log_status}")
                @logger.fatal("#{ip} is not faulty!") if !@vms[ip].faulty?
            end

            @vms[ip].status = :connected

            # While we have the lock, let's read some VM properties to use later
            is_faulty = @vms[ip].faulty?
        }

        begin
            # Keep making requests until some error occurs
            while @injected_faults < @faults.length

                # Send the faults to the faulty VM
                if is_faulty
                    step = "Sending faults..."

                    to_inject = @faults[@injected_faults .. (@injected_faults + FAULTS_PER_ROUND - 1)]
                    socket.puts("fault #{to_inject.join("|")}")
                    @logger.info("#{ip} faults sent (#{@injected_faults}.." \
                                 "#{@injected_faults + FAULTS_PER_ROUND - 1})")
                                # "\n\t#{to_inject.join("\n\t")}")
                    @injected_faults += to_inject.length

                    step = "Faults sent. Waiting for injection confirmation..."

                    Timeout::timeout(TIMEOUT) do
                        injected = socket.gets

                        if injected =~ /(\d+) faults injected/ && $1.to_i == FAULTS_PER_ROUND
                            @logger.info("#{ip} #{$1} faults injected")
                        else
                            throw "Unexpected value read from socket. Expected " \
                            "was 'faults injected', but received '#{injected}'"
                        end
                    end
                end

                step = "Waiting for the other VMs..."

                # Wait until all VM's are ready
                @barrier.wait

                step = "Starting the workload..."

                @logger.info("#{ip} starting workload")

                # Make the request
                socket.puts("execute")
                time_execute_sent = Time.now

                step = "Workload started! Receiving confirmation from the VM..."

                # Read the result and compare it with the correct one
                Timeout::timeout(TIMEOUT) do
                    started = socket.gets
                    if started =~ /started workload/
                        @logger.info("#{ip} started workload")
                    else
                        throw "Unexpected value read from socket. Expected " \
                        "was 'started workload', but received '#{started}'"
                    end

                    step = "Waiting for the workload result..."

                    result = socket.gets
                    throw "socket read nil result" if result.nil?

                    result = result.strip
                    throw "wrong result" if result != CORRECT_RESULT

                    @logger.info("#{ip} correct (#{sprintf("%.0f", Time.now - time_execute_sent)}s)")
                end

                step = "Waiting for fault removal confirmation..."

                # Wait until the faulty machine removes the previous faults
                if is_faulty
                    Timeout::timeout(TIMEOUT) do
                        removed = socket.gets

                        if removed =~ /(\d+) faults removed/ && $1.to_i == FAULTS_PER_ROUND
                            @logger.info("#{ip} #{$1} faults removed")
                        else
                            throw "Unexpected value read from socket. Expected " \
                            "was 'faults removed', but received '#{removed}'"
                        end
                    end
                end
            end

            @logger.info("Injection complete.")
        rescue Timeout::Error
            # receive timedout
            @logger.error("#{ip} timeout on: '#{step}'")
            reboot_vm(ip)
        rescue Exception => ex
            if !result.nil? && result != CORRECT_RESULT
                # incorrect result
                @logger.error("#{ip} wrong: '#{result}'")
            else
                # Some other kind of error ocurred... one which was not supposed to happen :-S
                @logger.error("#{ip}: #{ex}\n\t from: " + ex.backtrace.join("\n\t from: "))
            end
            reboot_vm(ip)
        end
    end

    def reboot_vm(ip)
        @vms_mutex.synchronize {
            if @vms[ip].faulty?
                output = @vms[ip].reboot()
                @logger.info("#{ip} reboot #{log_status}")
                @logger.error("#{ip} vmware-cmd error: #{output}") unless output == ""
            else
                @logger.fatal("#{ip} is not faulty! The machine is not going " \
                              "to be rebooted so that you can check what happened.")
            end
        }
    end

    def log_status
        status = "\n------------------------------------------------\n"
        status +=  "Hostname         | Status           | # reboots   \n"
        status +=  "-----------------+------------------+-----------\n"
        for value in @vms.values
            status += value.to_s + "\n"
        end
        status +=  "------------------------------------------------\n"
        status += "#{@injected_faults}/#{@faults.length} faults injected"
        status
    end
end


#
# Loads the configuration from the given YAML file
#
def load_configuration(file)
    File.open(file) do |fx|
        faults_file, port, vms_config = YAML::load_stream(fx).documents

        vms = {}
        vms_config.each { |k, v| vms[k] = VirtualMachine.new(v.hostname, v.vmx, v.product_server, v.product_username, v.faulty?) }

        return [faults_file, port, vms]
    end
end

if ARGV.length != 1
    puts "Usage: server <configuration>"
    exit
end

# load virtual machine settings
FAULTS_FILE, PORT, VMS = load_configuration(ARGV[0])

# read faults
faults = IO.readlines(FAULTS_FILE).map do |fault|
    fault.strip
end

# Create the logger
logger = Logger.new("#{ARGV[0].split(".").first}.log")
logger.datetime_format = "%a %Y/%m/%d %H:%M:%S"
logger.info("server started #{'#' * 80}")

server = FaultServer.new(PORT, VMS, faults, logger)
server.start
