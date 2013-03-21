MRuby::Gem::Specification.new('mruby-qrdecode') do |spec|
  spec.license = 'MIT'
  spec.authors = 'Carson McDonald'
  spec.version = '0.1.0'
  spec.description = 'mruby qr code decoder, uses quirc.'
  spec.homepage = 'https://github.com/carsonmcdonald/mruby-qrdecode'
 
  spec.cc.flags << "-I #{spec.dir}/src/quirc/lib/"

  spec.objs += Dir.glob("#{dir}/src/quirc/lib/*.{c,cpp,m,asm,S}").map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X.o") }
end
