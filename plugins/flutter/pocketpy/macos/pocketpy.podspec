#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint pocketpy.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'pocketpy'
  s.version          = '2.0.0'
  s.summary          = 'A new Flutter FFI plugin project.'
  s.description      = <<-DESC
A new Flutter FFI plugin project.
                       DESC
  s.homepage         = 'https://pocketpy.dev'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'blueloveTH' => 'blueloveth@foxmail.com' }

  # This will ensure the source files in Classes/ are included in the native
  # builds of apps using this FFI plugin. Podspec does not support relative
  # paths, so Classes contains a forwarder C file that relatively imports
  # `../src/*` so that the C sources can be shared among all target platforms.
  s.dependency 'FlutterMacOS'

  s.platform = :osx, '10.11'
  s.swift_version = '5.0'

  s.source                    = { :path => '.' }
  s.source_files              = 'Classes/**/*'
  s.library                   = 'c'
  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'OTHER_LDFLAGS' => '-force_load ' + __dir__ + '/pocketpy/build/libpocketpy.a',
  }

  s.prepare_command = <<-CMD
  rm -rf pocketpy
  git clone --branch v2.1.6 --depth 1 https://github.com/pocketpy/pocketpy.git
  cd pocketpy
  git submodule update --init --recursive --depth 1
  bash build_darwin_libs.sh
CMD
end

