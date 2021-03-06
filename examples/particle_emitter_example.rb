begin
  require 'rubygems'
rescue LoadError
end

$LOAD_PATH.unshift File.expand_path('../lib/', File.dirname(__FILE__))
require "ashton"

def media_path(file); File.expand_path "media/#{file}", File.dirname(__FILE__) end

class TestWindow < Gosu::Window
  def initialize
    super 640, 480, false
    self.caption = "Particle emitters"

    @grayscale = Ashton::Shader.new fragment: :grayscale

    @font = Gosu::Font.new self, Gosu::default_font_name, 24
    @star = Gosu::Image.new self, media_path("SmallStar.png"), true

    @image_emitter = Ashton::ParticleEmitter.new 450, 100, 0,
                                                 image: @star,
                                                 scale: 0.2,
                                                 speed: 20,
                                                 acceleration: -2,
                                                 max_particles: 5000,
                                                 interval: 0.005,
                                                 fade: 25, # loses 25 alpha/s
                                                 angular_velocity: -50..50

    @shaded_image_emitter = Ashton::ParticleEmitter.new 450, 350, 0,
                                                        image: @star,
                                                        shader: @grayscale,
                                                        interval: 0.001,
                                                        offset: 0..10,
                                                        max_particles: 5000,
                                                        angular_velocity: 20..50,
                                                        center_x: 3..8, center_y: 3..8,
                                                        zoom: -0.3 # Shrinks, so doesn't need TTL.

    @point_emitter = Ashton::ParticleEmitter.new 100, 100, 1,
                                                 scale: 10,
                                                 speed: 200,
                                                 interval: 0.0002,
                                                 max_particles: 5000,
                                                 interval: 0.003,
                                                 color: Gosu::Color.rgba(255, 0, 0, 150),
                                                 fade: 100 # loses 100 alpha/s

    @shaded_point_emitter = Ashton::ParticleEmitter.new 100, 300, 2,
                                                        scale: 4..10,
                                                        shader: @grayscale,
                                                        speed: 25..80,
                                                        offset: 0..10,
                                                        time_to_live: 12,
                                                        interval: 0.001,
                                                        max_particles: 5000,
                                                        color: Gosu::Color.rgba(255, 0, 0, 255),
                                                        gravity: 30 # pixels/s*s
  end

  def update
    $gosu_blocks.clear # workaround for Gosu 0.7.45 bug.

    @image_emitter.update        unless button_down? Gosu::Kb1
    @shaded_image_emitter.update unless button_down? Gosu::Kb2
    @point_emitter.update        unless button_down? Gosu::Kb3
    @shaded_point_emitter.update unless button_down? Gosu::Kb4
  end

  def button_down(id)
    if id == Gosu::KbEscape
      close
    end
  end

  def draw
    @point_emitter.draw
    @shaded_image_emitter.draw
    @shaded_point_emitter.draw
    @image_emitter.draw

    @font.draw "FPS: #{Gosu::fps} Pnt: #{@point_emitter.count} ShaPnt: #{@shaded_point_emitter.count} Img: #{@image_emitter.count} ShaImg: #{@shaded_image_emitter.count}", 0, 0, 0
  end
end

TestWindow.new.show