module Ashton
  class Texture
    include Mixins::VersionChecking

    DEFAULT_DRAW_COLOR = Gosu::Color::WHITE

    def rendering?; @rendering end

    def initialize(width, height)
      @rendering = false

      initialize_ width, height

      clear
    end

    public
    # Clears the buffer, optionally to a specific color.
    #
    # @option options :color [Gosu::Color, Array<Float>] (transparent)
    def clear(options = {})
      options = {
          color: [0.0, 0.0, 0.0, 0.0],
      }.merge! options

      color = options[:color]
      color = color.to_opengl if color.is_a? Gosu::Color

      glBindFramebufferEXT GL_FRAMEBUFFER_EXT, fbo_id unless rendering?

      glDisable GL_BLEND # Need to replace the alpha too.
      glClearColor *color
      glClear GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
      glEnable GL_BLEND

      glBindFramebufferEXT GL_FRAMEBUFFER_EXT, 0 unless rendering?

      nil
    end

    public
    # Enable the texture to use (e.g. to draw or convert it).
    def render
      raise ArgumentError, "block required (use #enable/#disable without blocks)" unless block_given?

      enable
      begin
        result = yield self
      ensure
        disable
      end

      result
    end

    public
    def enable
      raise AshtonError if rendering?

      $window.flush # Ensure that any drawing _before_ the render block is drawn to screen, rather than into the buffer.

      # Reset the projection matrix so that drawing into the buffer is zeroed.
      glBindFramebufferEXT GL_FRAMEBUFFER_EXT, fbo_id
      glPushMatrix
      glMatrixMode GL_PROJECTION
      glLoadIdentity
      glViewport 0, 0, width, height
      glOrtho 0,  width, height, 0, -1, 1

      @rendering = true
    end

    public
    def disable
      raise AshtonError unless rendering?

      $window.flush # Force all the drawing to draw now!
      glPopMatrix
      glBindFramebufferEXT GL_FRAMEBUFFER_EXT, 0

      @rendering = false
    end

    # @!method draw(x, y, z, options = {})
    #   Draw the image, _immediately_ (no z-ordering by Gosu).
    #
    #   This is not as versatile as converting the Texture into a Gosu::Image and then
    #   drawing it, but it is many times faster, so use it when you are updating the buffer
    #   every frame, rather than just composing an image.
    #
    #   Drawing in Gosu orientation will be flipped in standard OpenGL and visa versa.
    #
    #   @param x [Number] Top left corner x.
    #   @param y [Number] Top left corner y.
    #   @param z [Number] Z-order.
    #
    #   @option options :shader [Ashton::Shader] Shader to apply to drawing.
    #   @option options :color [Gosu::Color] (Gosu::Color::WHITE) Color to apply to the drawing.
    #   @option options :blend [Symbol] (:alpha) :alpha, :copy, :additive or :multiplicative

    def draw(x, y, z, options = {})
      shader = options[:shader]
      color = options[:color] || DEFAULT_DRAW_COLOR
      blend = options[:blend] || :alpha

      unless shader.nil? || shader.is_a?(Shader)
        raise TypeError, "Expected :shader option of type Ashton::Shader"
      end

      unless color.is_a? Gosu::Color
        raise TypeError, "Expected :color option of type Gosu::Color"
      end

      unless blend.is_a? Symbol
        raise TypeError, "Expected :blend option to be a Symbol"
      end

      shader.enable z if shader

      $window.gl z do
        if shader
          shader.color = color
          location = shader.send :uniform_location, "in_TextureEnabled", required: false
          shader.send :set_uniform, location, true if location != Shader::INVALID_LOCATION
        else
          glColor4f *color.to_opengl
        end

        glEnable GL_BLEND
        glEnable GL_TEXTURE_2D
        glBindTexture GL_TEXTURE_2D, id

        # Set blending mode.
        case
          when :default, :alpha
            glBlendFunc GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
          when :additive, :add
            glBlendFunc GL_ONE, GL_ONE
          when :multiplicative, :multiply
            glBlendFunc GL_DST_COLOR, GL_ZERO
          when :copy
            glBlendFunc GL_ONE, GL_ZERO
          else
            raise ArgumentError, "Unrecognised blend mode: #{options[:blend].inspect}"
        end

        glBegin GL_QUADS do
          glTexCoord2d 0, 0
          glVertex2d x, y + height # BL

          glTexCoord2d 0, 1
          glVertex2d x, y # TL

          glTexCoord2d 1, 1
          glVertex2d x + width, y # TR

          glTexCoord2d 1, 0
          glVertex2d x + width, y + height # BR
        end
      end

      shader.disable z if shader
    end

    public
    # Convert the current contents of the buffer into a Gosu::Image
    #
    # @option options :caching [Boolean] (true) TexPlay behaviour.
    # @option options :tileable [Boolean] (false) Standard Gosu behaviour.
    # @option options :rect [Array<Integer>] ([0, 0, width, height]) Rectangular area of buffer to use to create the image [x, y, w, h]
    def to_image(*args)
      cache.to_image *args
    end

    def dup
      new_texture = Texture.new width, height
      new_texture.render do
        draw 0, 0, 0, blend: :copy
      end
      new_texture
    end
  end
end