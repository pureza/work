require 'open3'

class VirtualMachine
  attr_reader :hostname, :reboots, :vmx, :product_server, :product_username
  attr_accessor :status
  
  def initialize(hostname, vmx, product_server, product_username, faulty = false)
    @hostname = hostname
    @vmx = vmx
    @product_server = product_server
    @product_username = product_username
    @faulty = faulty
    @reboots = 0
    @status = :disconnected
  end

  def reboot
    output = ""
    Open3.popen3("ssh #{@product_username}@#{@product_server} \"vmware-cmd '#{@vmx}' reset hard\"") do |stdin, stdout, stderr|
      output = stderr.readlines.join
    end
    
    @reboots += 1
    @status = :disconnected
    return output
  end

  def back
    @status = :connected
  end

  def faulty?
    @faulty
  end

  def to_s
    return sprintf("%-17s %-18s    %5d", @hostname, @status, @reboots)
  end
end
