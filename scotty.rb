class Scotty < Formula
  desc "Specify Characters on a TTY"
  homepage "https://github.com/Hardmath123/scotty"
  url "https://github.com/Hardmath123/scotty/archive/v1.0.0.tar.gz"
  version "1.0.0"
  sha256 "4058acb6b77234c952105c74684309241e8539b2ec353dfb47cfea1d9b7b4e86"

  def install
    system "make", "install", "PREFIX=#{prefix}",
                              "MANDIR=#{man}"
  end

  test do
    system "scotty"
  end
end
