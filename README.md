Description
====
The SpicePlay is a modified [SPICE](spice-space.org) client to add record/replay-based benchmarking support like [VNCplay](http://suif.stanford.edu/vncplay/) and [DeskBench](http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=5188870&url=http%3A%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D5188870). For the information about how the benchmarking works, refer to the above papers.

Install
====

The SpicePlay includes both server and client like original SPICE, so you should install SpicePlay in the server where KVM hypervisor runs, if you want to benchmark it using the SpicePlay client. Make sure the QEMU should be installed after SPICE server.

However, note that the SPICE version (0.8.0) of SpicePlay is outdated and installation requires several packages, which are likely outdated too. Refer to the official homepage of [SPICE](http://www.spice-space.org/) for up-to-date information. The following installation information would be used as reference. You do not have to follow this.

```
# apt-get install libtool liblog4cpp5-dev libavcodec-dev libssl-dev xlibmesa-glu-dev libasound-dev libpng12-dev libfreetype6-dev libfontconfig1-dev libogg-dev libxrandr-dev kvm libgcrypt-dev libsdl-dev libnss3-dev libpixman-1-dev libxfixes-dev libjpeg8-dev libsasl2-dev python-pyparsing

# git clone git://anongit.freedesktop.org/~alon/libcacard
# cd libcacard
# ./autogen.sh --includedir=/usr/include --libdir=/usr/lib64
# make
# make install
# cd ..

# wget http://downloads.us.xiph.org/releases/celt/celt-0.5.1.3.tar.gz
# tar xvzf celt-0.5.1.3.tar.gz
# cd celt-0.5.1.3/
# ./configure --includedir=/usr/include -- libdir=/usr/lib64
# make
# sudo make install
# cd ..

# git clone https://github.com/virtualAMP/spiceplay.git
# cd spiceplay
# apt-get install libsasl2-dev
# ./autogen.sh --includedir=/usr/include --libdir=/usr/lib64 --enable-benchmark --disable-werror    
# make
# make install

```

Usage
====
#### 1. Record
Since SpicePlay is based on record/replay, you should first record a trace of an interactive session. The way of recording in SpicePlay follows [DeskBench](http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=5188870&url=http%3A%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D5188870) in that you should explicitly mark a synchronization point (snapshot) by pressing a specific key, `PrtScn`. So, during the recording, you should press the PrtScn key on the screen of which you want to take a snapshot. The following command is used for recording.

```
# spicec -h <server address> -p <server port> -r <trace filename>
```

#### 2. Replay
Now that you have a trace recorded, you can replay and measure response time by using this trace. During the replay, it produces messages of how it proceeds to standard output and response time to standard error. So, you can use the following command to get response times.

```
# spicec -h <server address> -p <server port> -P <trace filename> 2> response_time.txt
```

You can insert think time between interactive episodes by using `-T <think time in msec>`. 

Misc
====
Note that this tool is to evaluate interactive performance for [vAMP paper](http://vee2014.cs.technion.ac.il/papers/VEE14-final23.pdf). As the main goal is fast-prototyping of benchmark-enabled remote desktop client, the codes are added in an ad-hoc manner. In addition, robust replay without failure is actually challenging in practice due to unexpected behaviors of GUI, as also mentioned in the previous papers ([VNCplay](http://suif.stanford.edu/vncplay/) and [DeskBench](http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=5188870&url=http%3A%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D5188870)). For more robust replaying, you should carefully record the session by avoiding complicated GUI behaviors like randomly popped tooltips. 

