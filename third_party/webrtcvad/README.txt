WebRTC's voice activity detector (common_audio/vad and the signal-processing routines it uses), plain C, as packaged
by py-webrtcvad (github.com/wiseman/py-webrtcvad, commit e283ca41df3a84b0e87fb1f5cb9b21580a286b09), unmodified; the
Python binding and the two C++ files it has are left out. Built into libsbdts (tools/build-ffmpeg.sh) with
-DWEBRTC_POSIX; rtc_FatalMessage is in tools/dts/sbdts.c. Licences: LICENSE-WebRTC-BSD.txt (WebRTC, BSD-3-Clause),
LICENSE-py-webrtcvad-MIT.txt (the packaging).
