#!/usr/bin/env ruby

require 'rand.rb'

if ARGV.length < 2
  puts "Usage: ./select_faults.rb <output file> <syscall1> <syscall2> ... <syscallN>"
  exit
end

SYSCALLS = ARGV[1..-1]
OUTPUT_FILE = ARGV[0]

# Reads all the unused faults
all_faults = []
for dir in SYSCALLS
  puts "Unknown syscall: #{dir}" if !File.exists? dir

  for fault_file in Dir["#{dir}/fault_*.txt"]
    puts "A ler as faults existentes em #{fault_file}..."
    all_faults += IO.readlines(fault_file)
  end
end

puts
puts "#{all_faults.length} faults encontradas"
puts "Eliminando faults duplicadas..."

# Removes repeated faults
no_repetitions = []
all_faults.each { |fault| no_repetitions << fault unless no_repetitions.include?(fault) }

puts "#{no_repetitions.length} faults únicas."
puts "A desordenar e guardar para o ficheiro #{OUTPUT_FILE}..."

# Writes scrambled and not repeated faults to the output
fx = File.new(OUTPUT_FILE, "w")
no_repetitions.shuffle.each { |fault| fx.puts fault }
fx.close

puts "done"

