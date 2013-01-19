mruby-qrdecode
==============

QR decoding for mruby using the [quirc library](https://github.com/dlbeer/quirc).

Depends on the [mruby-jpeg](https://github.com/carsonmcdonald/mruby-jpeg) library.

## Installation

In the mruby directory:

```bash
mkdir mrbgems
cd mrbgems
git clone --recursive git://github.com/carsonmcdonald/mruby-qrdecode.git
```

Then in your build config add:

```ruby
MRuby::Build.new do |conf|
  
  conf.gem 'mrbgems/mruby-qrdecode'

end
```

## Example Use

```ruby
input_jpeg=JPEG.read("/tmp/test.jpg", :force_grayscale => true)
qrd=QR::Decoder.new
puts qrd.decode(input_jpeg)
```

## License

MIT - See LICENSE
