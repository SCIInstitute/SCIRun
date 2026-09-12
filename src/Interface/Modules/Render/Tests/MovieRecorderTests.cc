/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <Interface/Modules/Render/MovieRecorder.h>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#ifndef WIN32
#include <unistd.h>
#endif

using namespace SCIRun::Gui;

namespace
{
  MovieSettings settingsFor(const QString& path, MovieFormat format = MovieFormat::PngSequence)
  {
    MovieSettings s;
    s.outputPath = path;
    s.format = format;
    s.frameRate = 30;
    return s;
  }
}

TEST(MovieSettingsTest, SuffixMatchesFormat)
{
  EXPECT_EQ("png", settingsFor("a", MovieFormat::PngSequence).suffix().toStdString());
  EXPECT_EQ("mp4", settingsFor("a", MovieFormat::Mp4).suffix().toStdString());
  EXPECT_EQ("gif", settingsFor("a", MovieFormat::Gif).suffix().toStdString());
}

TEST(MovieSettingsTest, OnlyVideoFormatsNeedAnEncoder)
{
  EXPECT_FALSE(settingsFor("a", MovieFormat::PngSequence).needsEncoder());
  EXPECT_TRUE(settingsFor("a", MovieFormat::Mp4).needsEncoder());
  EXPECT_TRUE(settingsFor("a", MovieFormat::Gif).needsEncoder());
}

TEST(MovieSettingsTest, FormatRoundTripsThroughItsSuffix)
{
  for (auto format : {MovieFormat::PngSequence, MovieFormat::Mp4, MovieFormat::Gif})
    EXPECT_EQ(format, movieFormatFromSuffix(settingsFor("a", format).suffix()));

  EXPECT_EQ(MovieFormat::Mp4, movieFormatFromSuffix("MP4"));
  EXPECT_EQ(MovieFormat::PngSequence, movieFormatFromSuffix("avi"));
  EXPECT_EQ(MovieFormat::PngSequence, movieFormatFromSuffix(""));
}

TEST(MovieRecorderValidationTest, AcceptsAWritableDestination)
{
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  EXPECT_TRUE(MovieRecorder::validate(settingsFor(dir.path() + "/movie.png")).isEmpty());
}

TEST(MovieRecorderValidationTest, RejectsEmptyPath)
{
  EXPECT_FALSE(MovieRecorder::validate(settingsFor("")).isEmpty());
  EXPECT_FALSE(MovieRecorder::validate(settingsFor("   ")).isEmpty());
}

TEST(MovieRecorderValidationTest, RejectsNonsenseFrameRate)
{
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  auto settings = settingsFor(dir.path() + "/movie.png");
  settings.frameRate = 0;
  EXPECT_FALSE(MovieRecorder::validate(settings).isEmpty());
}

// The v4 recorder wrote nothing and said nothing when the directory was
// missing (issue #279); it is created up front now.
TEST(MovieRecorderValidationTest, CreatesAMissingOutputDirectory)
{
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const auto nested = dir.path() + "/frames/run1";
  ASSERT_FALSE(QDir(nested).exists());

  EXPECT_TRUE(MovieRecorder::validate(settingsFor(nested + "/movie.png")).isEmpty());
  EXPECT_TRUE(QDir(nested).exists());
}

TEST(MovieRecorderValidationTest, RejectsUnwritableDirectory)
{
#ifdef WIN32
  GTEST_SKIP() << "POSIX directory permissions do not apply.";
#else
  if (geteuid() == 0)
    GTEST_SKIP() << "root ignores directory permissions.";

  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const auto locked = dir.path() + "/locked";
  ASSERT_TRUE(QDir().mkpath(locked));
  ASSERT_TRUE(QFile::setPermissions(locked,
    QFileDevice::ReadOwner | QFileDevice::ExeOwner));

  EXPECT_FALSE(MovieRecorder::validate(settingsFor(locked + "/movie.png")).isEmpty());

  // Restore so QTemporaryDir can clean up after itself.
  QFile::setPermissions(locked,
    QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
#endif
}

TEST(MovieRecorderValidationTest, RejectsVideoFormatsWithoutFfmpeg)
{
  if (MovieRecorder::ffmpegAvailable())
    GTEST_SKIP() << "ffmpeg is installed here, so video formats are valid.";

  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const auto problem = MovieRecorder::validate(settingsFor(dir.path() + "/movie.mp4", MovieFormat::Mp4));
  EXPECT_TRUE(problem.contains("ffmpeg"));
}
