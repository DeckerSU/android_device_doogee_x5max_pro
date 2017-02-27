/*
 * Copyright (C) 2012 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.romstats;

import java.io.File;
import java.math.BigInteger;
import java.net.NetworkInterface;
import java.security.MessageDigest;
import java.util.Locale;

import android.content.ComponentName;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Environment;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class Utilities {
	public static final String SETTINGS_PREF_NAME = "ROMStats";
	public static final int NOTIFICATION_ID = 1;

	// For the Unique ID, I still use the IMEI or WiFi MAC address
	// CyanogenMod switched to use the Settings.Secure.ANDROID_ID
	// This is because the ANDROID_ID could change on hard reset, while IMEI remains equal
	public static String getUniqueID(Context ctx) {
		TelephonyManager tm = (TelephonyManager) ctx.getSystemService(Context.TELEPHONY_SERVICE);

		String device_id = digest(tm.getDeviceId());
		if (device_id == null) {
			String wifiInterface = SystemProperties.get("wifi.interface");
			try {
				String wifiMac = new String(NetworkInterface.getByName(wifiInterface).getHardwareAddress());
				device_id = digest(wifiMac);
			} catch (Exception e) {
				device_id = null;
			}
		}

		return device_id;
	}

	public static String getStatsUrl() {
		// String returnUrl = SystemProperties.get("ro.romstats.url");
		String returnUrl = "http://roms.decker.su/";

		if (returnUrl.isEmpty()) {
			return null;
		}

		// if the last char of the link is not /, add it
		if (!returnUrl.substring(returnUrl.length() - 1).equals("/")) {
			returnUrl += "/";
		}

		return returnUrl;
	}

	public static String getCarrier(Context ctx) {
		TelephonyManager tm = (TelephonyManager) ctx.getSystemService(Context.TELEPHONY_SERVICE);
		String carrier = tm.getNetworkOperatorName();
		if ("".equals(carrier)) {
			carrier = "Unknown";
		}
		return carrier;
	}

	public static String getCarrierId(Context ctx) {
		TelephonyManager tm = (TelephonyManager) ctx.getSystemService(Context.TELEPHONY_SERVICE);
		String carrierId = tm.getNetworkOperator();
		if ("".equals(carrierId)) {
			carrierId = "0";
		}
		return carrierId;
	}

	public static String getCountryCode(Context ctx) {
		TelephonyManager tm = (TelephonyManager) ctx.getSystemService(Context.TELEPHONY_SERVICE);
		String countryCode = tm.getNetworkCountryIso();
		if (countryCode.equals("")) {
			countryCode = "Unknown";
		}
		return countryCode;
	}

	public static String getDevice() {
		return SystemProperties.get("ro.product.model");
	}

	public static String getModVersion() {
		return SystemProperties.get("ro.build.display.id");
	}

	public static String getRomName() {
		return SystemProperties.get("ro.romstats.name");
	}

	public static String getRomVersion() {
		// return "5.7.3";
		   return SystemProperties.get("ro.romstats.version","Unknown");
	}

	public static String getRomVersionHash() {
		String romHash = getRomName() + getRomVersion();
		return digest(romHash);
	}

	public static long getTimeFrame() {
		String tFrameStr = SystemProperties.get("ro.romstats.tframe", "7");
		return Long.valueOf(tFrameStr);
	}

	public static String digest(String input) {
		try {
			MessageDigest md = MessageDigest.getInstance("MD5");
			return new BigInteger(1, md.digest(input.getBytes())).toString(16).toUpperCase(Locale.US);
		} catch (Exception e) {
			return null;
		}
	}

	
	/**
	 * Gets the Ask First value
	 * 0: RomStats will behave like CMStats, starts reporting automatically after the tframe (default)
	 * 1: RomStats will behave like the old CMStats, asks the user on first boot
	 *
	 * @return boolean
	 */
	public static int getReportingMode() {
			return 0;
	}

	/**
	 *
	 * @param context
	 * @return
	 * 	false: opt out cookie not present, work normally
	 * 	true: opt out cookie present, disable and close
	 */
	public static boolean persistentOptOut(Context context) {
		SharedPreferences prefs = AnonymousStats.getPreferences(context);

		Log.d(Const.TAG, "[checkPersistentOptOut] Check prefs exist: " + prefs.contains(Const.ANONYMOUS_OPT_IN));
		if (!prefs.contains(Const.ANONYMOUS_OPT_IN)) {
			Log.d(Const.TAG, "[checkPersistentOptOut] New install, check for 'Persistent cookie'");

			File sdCard = Environment.getExternalStorageDirectory();
			File dir = new File (sdCard.getAbsolutePath() + "/.ROMStats");
			File cookieFile = new File(dir, "optout");

			if (cookieFile.exists()) {
				// if cookie exists, disable everything by setting:
				//   OPT_IN = false
				//   FIRST_BOOT = false
				Log.d(Const.TAG, "[checkPersistentOptOut] Persistent cookie exists -> Disable everything");

				prefs.edit().putBoolean(Const.ANONYMOUS_OPT_IN, false).apply();
				prefs.edit().putBoolean(Const.ANONYMOUS_FIRST_BOOT, false).apply();

				SharedPreferences mainPrefs = PreferenceManager.getDefaultSharedPreferences(context);
				mainPrefs.edit().putBoolean(Const.ANONYMOUS_OPT_IN, false).apply();
				mainPrefs.edit().putBoolean(Const.ANONYMOUS_OPT_OUT_PERSIST, true).apply();

				return true;
			} else {
				Log.d(Const.TAG, "[checkPersistentOptOut] No persistent cookie found");
			}
		};

		return false;
	}

	public static void checkIconVisibility(Context context) {
		File sdCard = Environment.getExternalStorageDirectory();
		File dir = new File (sdCard.getAbsolutePath() + "/.ROMStats");
		File cookieFile = new File(dir, "hide_icon");

		PackageManager p = context.getPackageManager();
		ComponentName componentToDisable = new ComponentName("android.romstats", "android.romstats.AnonymousStats");
		if (cookieFile.exists()) {
			// exist, hide icon
			p.setComponentEnabledSetting(componentToDisable, PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);
		} else {
			// does not exist, show icon
			p.setComponentEnabledSetting(componentToDisable, PackageManager.COMPONENT_ENABLED_STATE_ENABLED, PackageManager.DONT_KILL_APP);
		}
	}


}
