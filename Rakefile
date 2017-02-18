require "bundler/gem_tasks"
require "rake/testtask"

Rake::TestTask.new(:test) do |t|
  t.libs << "test"
  t.libs << "lib"
  t.test_files = FileList['test/**/*_test.rb']
end

require "rake/extensiontask"

task :build => :compile

Rake::ExtensionTask.new("penguin_queue") do |ext|
  ext.lib_dir = "lib/penguin_queue"
end

task :default => [:clobber, :compile, :test]
