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
TIMEOUT = 40

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
          @logger.info("CONNECTED\t\t #{ip}")
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
    result = ""
    ip = socket.peeraddr[3]
    is_faulty = false

    # Change the VM status to connected
    @vms_mutex.synchronize {
      throw "Unknown VM connection from '#{ip}'" unless @vms.has_key?(ip)

      if @vms[ip].status == :disconnected
        @vms[ip].status = :connected
      else
        throw "The VM '#{@vms[ip].hostname}' is already connected!"
      end

      # While we have the lock, let's read some VM properties to use later
      is_faulty = @vms[ip].faulty?
    }
 
    begin
      # Keep making requests until some error occurs
      while @injected_faults < @faults.length

        # Send the faults to the faulty VM
        if is_faulty
          to_inject = @faults[@injected_faults .. (@injected_faults + FAULTS_PER_ROUND - 1)]
          socket.puts("FAULT #{to_inject.join("|")}")
          @logger.info("INJECTION\t\t #{ip} ##{@injected_faults + 1}\n\t#{to_inject.join("\n\t")}")
          @injected_faults += to_inject.length
        end

        # Wait until all VM's are ready
        @barrier.wait

        # Make the request
        socket.puts("EXECUTE")
        time_execute_sent = Time.now

        # Read the result and compare it with the correct one
        Timeout::timeout(is_faulty ? TIMEOUT : 0) do
          result = socket.gets
          throw "socket read nil result" if result.nil?

          result = result.strip
          throw "wrong result" if result != CORRECT_RESULT

          @logger.info("DURATION:\t\t #{ip} #{sprintf("%.0f", Time.now - time_execute_sent)}s")
        end
      end
      @logger.info("Injection complete.")
    rescue Timeout::Error
      # receive timedout
      @logger.error("TIMEOUT\t\t #{ip}")
      reboot_vm(ip)
    rescue Exception => ex
      if !result.nil? && result != CORRECT_RESULT
        # incorrect result
        @logger.error("WRONG\t\t #{ip} #{result}")
      else
        # Some other kind of error ocurred... one which was not supposed to happen :-S
        @logger.error("#{ip}: #{ex}\n\t from: " + ex.backtrace.join("\n\t from: "))
      end
      reboot_vm(ip)
    end
  end

  def reboot_vm(ip)
    @vms_mutex.synchronize {
      output = @vms[ip].reboot()
      @logger.info("RESTART\t\t #{ip}#{log_status}")
      @logger.error("VMWARE-CMD ERROR\t #{ip}: #{output}") unless output == ""
      @logger.fatal("#{ip} is not faulty!") if !@vms[ip].faulty?
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
logger.info("SESSION START")

server = FaultServer.new(PORT, VMS, faults, logger)
server.start
