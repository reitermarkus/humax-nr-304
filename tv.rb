#!/usr/bin/env ruby
# frozen_string_literal: true

require 'yaml'
require 'json'
require 'open3'

CONFIG = {
  type:  'NEC',
  bits:  32,
  codes: {
    power_toggle: 0x100FF,
    tv_av:        0x10AF5,
    zoom:         0x10EF1,
    menu:         0x1F00F,
    pic_mode:     0x118E7,
    sound_effect: 0x19867,
    mute:         0x1E817,
    num_1:        0x1C03F,
    num_2:        0x120DF,
    num_3:        0x1A05F,
    num_4:        0x1906F,
    num_5:        0x1E01F,
    num_6:        0x110EF,
    num_7:        0x1906F,
    num_8:        0x150AF,
    num_9:        0x1D02F,
    list:         0x1B24D,
    num_0:        0x130CF,
    last:         0x104FB,
    red:          0x138C7,
    green:        0x1B847,
    yellow:       0x158A7,
    blue:         0x17887,
    epg:          0x1D827,
    info:         0x1C23D,
    opt_plus:     0x142BD,
    back:         0x1827D,
    up:           0x18877,
    down:         0x1A857,
    left:         0x148B7,
    right:        0x128D7,
    enter:        0x1C837,
    vol_minus:    0x102FD,
    vol_plus:     0x1F807,
    ch_minus:     0x152AD,
    ch_plus:      0x1926D,
    page_down:    0x144BB,
    page_up:      0x1847B,
    tv_menu:      0x1708F,
    sleep:        0x14EB1,
    i_plus_ii:    0x1A25D,
    dtv_radio:    0x14AB5,
    pip:          0x16E91,
    pip_p_plus:   0x1DE21,
    pip_p_minus:  0x13EC1,
    pip_input:    0x1EE11,
    pip_position: 0x15EA1,
    pip_swap:     0x11EE1,
    pip_matrix_1: 0x106F9,
    pip_matrix_2: 0x19669,
    pip_matrix_3: 0x126D9,
    pip_matrix_4: 0x1A659,
    pip_matrix_5: 0x1E619,
    pip_matrix_6: 0x18679,
    pip_matrix_7: 0x146B9,
    pip_matrix_8: 0x1C639,
    pip_matrix_9: 0x16699,
    factory_menu: 0x109f6,
  },
}

HOSTNAME, *COMMANDS = ARGV

URL = "http://#{HOSTNAME}/sendir"

def sendir(code, bits, type)
  out, err, status = Open3.capture3(
    'curl',
    '--fail', '--data', { data: code, bits: bits, type: type }.to_json,
    '--header', 'Content-Type: application/json', URL,
  )

  raise "#{out}\n#{err}" unless status.success?
end

if COMMANDS.first == 'test'
  (0x109f7..0x20000).each do |code|
    if CONFIG[:codes].values.include?(code)
      puts "skipped 0x#{code.to_s(16)}"
      next
    end

    puts "sending 0x#{code.to_s(16)}"
    sendir(code, CONFIG[:bits], CONFIG[:type])
  end
else
  COMMANDS.each do |command|
    code = CONFIG[:codes].fetch(command.downcase.to_sym)

    sendir(code, CONFIG[:bits], CONFIG[:type])
  end
end
