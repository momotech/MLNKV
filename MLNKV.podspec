#
#  Be sure to run `pod spec lint MLNKV.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see https://guides.cocoapods.org/syntax/podspec.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |spec|

  spec.name         = "MLNKV"
  spec.version      = "0.0.2"
  spec.summary      = "MLNKV is a key-value storage framework ."

  spec.description  = <<-DESC
                    MLNKV is a key-value storage framework !!
                   DESC

  spec.homepage     = "https://github.com/momotech/MLNKV"
  spec.license      = { :type => "MIT", :file => "FILE_LICENSE" }
  spec.author       = ""

  spec.ios.deployment_target = "9.0"
  spec.requires_arc = true

  spec.source       = { :git => "https://github.com/momotech/MLNKV.git", :tag => spec.version.to_s }

  spec.source_files  = "Source/iOS/*.{h,m,mm}"
  spec.public_header_files = "Source/iOS/*.h"

  spec.subspec 'cpp' do |ss|
    ss.source_files = "Source/cpp/*.{h,hpp,cpp}"
    ss.public_header_files = "Source/cpp/*.{h,hpp}"
  end

  spec.ios.frameworks = "UIKit"

  spec.libraries = "z", "c++"
  spec.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++11',
    'CLANG_CXX_LIBRARY' => 'libc++'
  }

end
