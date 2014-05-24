/*
 * Copyright (c) 2013 Carson McDonald
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal 
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>

#include <string.h>
#include <stdio.h>

#include "quirc/lib/quirc.h"

static void
mrb_sd_qr_decode_free(mrb_state *mrb, void *decoder)
{
  if(decoder)
  {
    quirc_destroy(decoder);
  }
}

static const struct mrb_data_type mrb_qr_decoder_type = { "qr_decoder", mrb_sd_qr_decode_free };

static mrb_value
mrb_qr_decoder_init(mrb_state *mrb, mrb_value self)
{
  struct quirc *decoder = quirc_new();
  if (!decoder) 
  {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not allocate decoder.");
  }

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "decoder"), mrb_obj_value(Data_Wrap_Struct(mrb, mrb->object_class, &mrb_qr_decoder_type, (void*)decoder)));

  return self;
}

static mrb_value
mrb_qr_decoder_decode(mrb_state *mrb, mrb_value self)
{
  mrb_value value_decoder = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "decoder"));

  struct quirc *decoder = mrb_get_datatype(mrb, value_decoder, &mrb_qr_decoder_type);
  if(!decoder)
  {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Decoder instance variable not set.");
  }

  mrb_value value_image = mrb_nil_value();
  mrb_get_args(mrb, "o", &value_image);
  if(mrb_nil_p(value_image))
  {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Invalid argument.");
  }

  mrb_value value_width = mrb_iv_get(mrb, value_image, mrb_intern_lit(mrb, "width"));
  mrb_value value_height = mrb_iv_get(mrb, value_image, mrb_intern_lit(mrb, "height"));

  if(quirc_resize(decoder, mrb_fixnum(value_width), mrb_fixnum(value_height)) < 0)
  {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not resize decoder.");
  }

  uint8_t *image_buffer = quirc_begin(decoder, NULL, NULL);

  mrb_value value_data = mrb_iv_get(mrb, value_image, mrb_intern_lit(mrb, "data"));
  memcpy(image_buffer, RSTRING_PTR(value_data), RSTRING_LEN(value_data));
 
  quirc_end(decoder);

  mrb_value results = mrb_ary_new(mrb);
 
  int qrcount = quirc_count(decoder);
  int i = 0;
  for(i = 0; i < qrcount; i++) 
  {
    struct quirc_code code;
    quirc_extract(decoder, i, &code);

    struct quirc_data data;
    quirc_decode_error_t err = quirc_decode(&code, &data);

    mrb_value result = mrb_hash_new(mrb);
    if (err) 
    {
      const char *error = quirc_strerror(err);
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "error"), mrb_str_new_cstr(mrb, error));
    } 
    else 
    {
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "version"), mrb_fixnum_value(data.version));
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "ecc_level"), mrb_fixnum_value("MLHQ"[data.ecc_level]));
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "mask"), mrb_fixnum_value(data.mask));
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "data_type"), mrb_fixnum_value(data.data_type));
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "length"), mrb_fixnum_value(data.payload_len));
      mrb_hash_set(mrb, result, mrb_str_new_cstr(mrb, "payload"), mrb_str_new_cstr(mrb, (const char *)data.payload));
    }

    mrb_ary_push(mrb, results, result);
  }

  return results;
}

void
mrb_mruby_qrdecode_gem_init(mrb_state* mrb) 
{
  struct RClass *module_qr = mrb_define_module(mrb, "QR");
  struct RClass *class_decoder = mrb_define_class_under(mrb, module_qr, "Decoder", mrb->object_class);

  mrb_define_method(mrb, class_decoder, "initialize", mrb_qr_decoder_init, ARGS_NONE());
  mrb_define_method(mrb, class_decoder, "decode", mrb_qr_decoder_decode, ARGS_REQ(1));
}

void
mrb_mruby_qrdecode_gem_final(mrb_state* mrb) 
{ 
}
