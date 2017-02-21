# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'penguin_queue/version'

Gem::Specification.new do |spec|
  spec.name          = "penguin_queue"
  spec.version       = PenguinQueue::VERSION
  spec.authors       = ["tompng"]
  spec.email         = ["tomoyapenguin@gmail.com"]

  spec.summary       = %q{C Ext Priority Queue}
  spec.description   = %q{C Ext Priority Queue (binary heap)}
  spec.homepage      = "https://github.com/tompng/penguin_queue"
  spec.license       = "MIT"
  spec.required_ruby_version = '>= 2.2.0'
  spec.extensions    = %w[ext/penguin_queue/extconf.rb]

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.13"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "minitest", "~> 5.0"
  spec.add_development_dependency "rake-compiler"
end
