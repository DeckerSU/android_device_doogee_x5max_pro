###Качество видео (MTK):


* pref_video_quality_entry_low_cam - низкое, 144x176 (3GP)
* pref_video_quality_entry_medium_cam - среднее. 320x480 (3GP)
* pref_video_quality_entry_high_cam - высокое. 480x640 (3GP)
* pref_video_quality_entry_fine_cam - хорошее. 720x1280 (3GP)

vendor/mediatek/proprietary/frameworks/base/media/camcorder/java/com/mediatek/camcorder/**CamcorderProfileEx.java ** - пролема в том, что эти значения лежат в классе CamcorderProfileEx и я не знаю как "прицепить" его к стандарному CamcorderProfile в Cyanogen. Менять API Cyanogen'а **нельзя**!

CamcorderProfileEx Values:

      	   public static final int QUALITY_LOW = 108;
           public static final int QUALITY_MEDIUM = 109;
           public static final int QUALITY_HIGH = 110;
           public static final int QUALITY_FINE = 111;
           public static final int QUALITY_NIGHT_LOW = 112;
           public static final int QUALITY_NIGHT_MEDIUM = 113;
           public static final int QUALITY_NIGHT_HIGH = 114;
           public static final int QUALITY_NIGHT_FINE = 115;
           public static final int QUALITY_LIVE_EFFECT = 116;
           public static final int QUALITY_H264_HIGH   = 117;
           public static final int QUALITY_FINE_4K2K       = 123;
           public static final int SLOW_MOTION_VGA_120FPS = 2231;
           public static final int SLOW_MOTION_HD_60FPS = 2240;
           public static final int SLOW_MOTION_HD_120FPS = 2241;
           public static final int SLOW_MOTION_HD_180FPS = 2242;
           public static final int SLOW_MOTION_HD_240FPS = 2243;
           public static final int SLOW_MOTION_FHD_60FPS = 2250;
           public static final int SLOW_MOTION_FHD_120FPS = 2251;
           public static final int SLOW_MOTION_FHD_240FPS = 2252;


MTK: packages/apps/Camera/res/values/arrays.xml 

	    <!-- Camera Preferences Video Quality entries -->
	    <string-array name="pref_video_quality_entries" translatable="false">
	        <item>@string/pref_video_quality_entry_low_cam</item>
	        <item>@string/pref_video_quality_entry_medium_cam</item>
	        <item>@string/pref_video_quality_entry_high_cam</item>
	        <item>@string/pref_video_quality_entry_fine_cam</item>
	        <item>@string/pref_video_quality_entry_fine_4k2k</item>
	        <item>@string/pref_video_quality_entry_1080p</item>
	    </string-array>
	    <string-array name="pref_video_quality_entryvalues" translatable="false">
	        <!-- The integer value of CamcorderProfileEx.QUALITY_LOW -->
	        <item>108</item>
	         <!-- The integer value of CamcorderProfileEx.QUALITY_MEDIUM -->
	        <item>109</item>
	        <!-- The integer value of CamcorderProfileEx.QUALITY_HIGH -->
	        <item>110</item>
	        <!-- The integer value of CamcorderProfileEx.QUALITY_FINE -->
	        <item>111</item>
	        <!-- The integer value of CamcorderProfileEx.QUALITY_FINE_4k2k -->
	        <item>123</item>
	        <!-- The integer value of CamcorderProfileEx.QUALITY_1080P -->
	        <item>118</item>
	    </string-array>
	
MTK: Camera/src/com/mediatek/camera/setting/SettingGenerator.java 	
	

	    private ArrayList<String> getMTKSupportedVideoQuality() {
	        ArrayList<String> supported = new ArrayList<String>();
	        // Check for supported quality, pip mode always check main camera's
	        // quality
	        int cameraId = /*
	                        * mContext.isCurrentPIPEnable() ?
	                        * CameraHolder.instance().getBackCameraId() :
	                        */mCameraId;
	        if (CamcorderProfile.hasProfile(cameraId, CamcorderProfileEx.QUALITY_LOW)) {
	            if(isCamcorderProfileInVideoSizes(cameraId, CamcorderProfileEx.QUALITY_LOW))
	                supported.add(VIDEO_QUALITY_LOW);
	        }
	        if (CamcorderProfile.hasProfile(cameraId, CamcorderProfileEx.QUALITY_MEDIUM)) {
	            if(isCamcorderProfileInVideoSizes(cameraId, CamcorderProfileEx.QUALITY_MEDIUM))
	                supported.add(VIDEO_QUALITY_MEDIUM);
	        }
	        if (CamcorderProfile.hasProfile(cameraId, CamcorderProfileEx.QUALITY_HIGH)) {
	            if(isCamcorderProfileInVideoSizes(cameraId, CamcorderProfileEx.QUALITY_HIGH))
	                supported.add(VIDEO_QUALITY_HIGH);
	        }
	        if (CamcorderProfile.hasProfile(cameraId, CamcorderProfileEx.QUALITY_FINE)) {
	            if(isCamcorderProfileInVideoSizes(cameraId, CamcorderProfileEx.QUALITY_FINE))
	                supported.add(VIDEO_QUALITY_FINE);
	        }
	        if (CamcorderProfile.hasProfile(cameraId, CamcorderProfileEx.QUALITY_FINE_4K2K)) {
	            List<Size> sizes = mICameraDevice.getParameters().getSupportedVideoSizes();
	            Iterator<Size> it = sizes.iterator();
	            boolean support = false;
	            while (it.hasNext()) {
	                Size size = it.next();
	                if (size.width >= VIDEO_2K42_WIDTH) {
	                    support = true;
	                    isSupport4K2K = true;
	                    break;
	                }
	            }
	            if (support) {
	                supported.add(VIDEO_QUALITY_FINE_4K2K);
	            }
	        }
	
	        return supported;
	    }

И еще:

	    // whether the profile video width/height is in video sizes, if not, record will error
	    private boolean isCamcorderProfileInVideoSizes(int cameraId, int camcorderProfileId) {
	        if(!FeatureSwitcher.isTablet()) {   // not tablet
	            return true;
	        }
	
	        boolean exist = false;
	        Parameters parameters = mICameraDevice.getParameters();
	        List<Size> supportedVideoSizes = parameters.getSupportedVideoSizes();
	
	        CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraId, camcorderProfileId);
	        int videoWidth = profile.videoFrameWidth;
	        int videoHeight = profile.videoFrameHeight;
	
	        for(Iterator it = supportedVideoSizes.iterator(); it.hasNext();){
	            Size size = (Size)it.next();
	                if(size.width == videoWidth && size.height == videoHeight){
	                    Log.d(TAG, "camcorder profile:" + camcorderProfileId +
	                        " exist: " + "width=" + videoWidth + ",height=" + videoHeight);
	                    exist = true;
	                    break;
	                }
	        }
	
	        if(!exist) {
	            Log.w(TAG, "CamcorderProfileId " + camcorderProfileId
	                     + ", width:" + videoWidth + ", height:" + videoHeight
	                     + " Not Exist In Feature Table!");
	        }
	
	        return exist;
	    }
	
	
