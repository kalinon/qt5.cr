module Qt
  class Application
    @argv : UInt8** = ARGV_UNSAFE
    @argc : Int32 = ARGC_UNSAFE

    def initialize
      @unwrap = Binding.bg_QApplication__CONSTRUCT_int_R_char_XX_int(pointerof(@argc), @argv, QT_VERSION)
      Qt.qt5cr_reset_numeric_locale
    end

    # Create an application object and pass it to the block.
    #
    # The application is started (by calling #exec) after the block
    # returns. Use this method to prevent the application object to be
    # deleted accidentally by the garbage collector.
    def self.exec(**args, &block)
      app = Application.new(**args)
      yield(app)
      Application.exec
      app # this ensures that the application is not removed by the GC
    end
  end
end
