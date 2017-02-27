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
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.text.DateFormat;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.NotificationManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.Signature;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

public class AnonymousStats extends PreferenceActivity implements
		DialogInterface.OnClickListener, DialogInterface.OnDismissListener,
		Preference.OnPreferenceChangeListener {

	private static final String PREF_VIEW_STATS = "pref_view_stats";
	private static final String PREF_LAST_REPORT_ON = "pref_last_report_on";
	private static final String PREF_REPORT_INTERVAL = "pref_reporting_interval";
	private static final String PREF_ABOUT = "pref_about";
	private static final String PREF_WEBSITE = "pref_website";

	private CheckBoxPreference mEnableReporting;
	private CheckBoxPreference mPersistentOptout;
	private Preference mViewStats;
	//private Preference btnUninstall;

	private Dialog mOkDialog;
	private boolean mOkClicked;

	private SharedPreferences mPrefs;

	public static SharedPreferences getPreferences(Context context) {
		return context.getSharedPreferences(Utilities.SETTINGS_PREF_NAME, 0);
	}

	@SuppressWarnings("deprecation")
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		addPreferencesFromResource(R.xml.anonymous_stats);

		mPrefs = getPreferences(this);

		PreferenceScreen prefSet = getPreferenceScreen();
		mPrefs = this.getSharedPreferences(Utilities.SETTINGS_PREF_NAME, 0);
		mEnableReporting = (CheckBoxPreference) prefSet.findPreference(Const.ANONYMOUS_OPT_IN);
		mEnableReporting.setEnabled(false);
		mPersistentOptout = (CheckBoxPreference) prefSet.findPreference(Const.ANONYMOUS_OPT_OUT_PERSIST);
		mPersistentOptout.setEnabled(false);
		mViewStats = (Preference) prefSet.findPreference(PREF_VIEW_STATS);
		//btnUninstall = prefSet.findPreference(PREF_UNINSTALL);
		mPrefs.edit().putBoolean(Const.ANONYMOUS_OPT_IN, true).apply();

		boolean firstBoot = mPrefs.getBoolean(Const.ANONYMOUS_FIRST_BOOT, true);
        if (firstBoot) {
        	Log.d(Const.TAG, "First app start, set params and report immediately");
            mPrefs.edit().putBoolean(Const.ANONYMOUS_FIRST_BOOT, false).apply();
            mPrefs.edit().putLong(Const.ANONYMOUS_LAST_CHECKED, 1).apply();
            ReportingServiceManager.launchService(this);
        }

		Preference mPrefAboutVersion = (Preference) prefSet.findPreference(PREF_ABOUT);
		String versionString = getResources().getString(R.string.app_name);
		try {
			versionString += " v" + getPackageManager().getPackageInfo(getBaseContext().getPackageName(), 0).versionName;
		} catch (Exception e) {
			// nothing
		}

		// mPrefAboutVersion.setTitle(versionString);
		mPrefAboutVersion.setTitle(getResources().getString(R.string.pref_info_about));
		mPrefAboutVersion.setSummary(versionString);

		Preference aboutWesbite = (Preference) prefSet.findPreference(PREF_WEBSITE);
		aboutWesbite.setOnPreferenceClickListener(new OnPreferenceClickListener() {
			@Override
			public boolean onPreferenceClick(Preference preference) {
    			Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(getResources().getString(R.string.pref_info_website_url)));
    			startActivity(intent);

				return false;
			}
		});

		Preference mPrefHolder;
		/* Experimental feature 2 */
		Long lastCheck = mPrefs.getLong(Const.ANONYMOUS_LAST_CHECKED, 0);
		if (lastCheck > 1) {
			// show last checkin date
			String lastCheckStr = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.SHORT).format(new java.util.Date(lastCheck));
			lastCheckStr = getResources().getString(R.string.last_report_on) + ": " + lastCheckStr;

			mPrefHolder = prefSet.findPreference(PREF_LAST_REPORT_ON);
			mPrefHolder.setTitle(lastCheckStr);

			Long nextCheck = mPrefs.getLong(Const.ANONYMOUS_NEXT_ALARM, 0);
			if (nextCheck > 0) {
				String nextAlarmStr = DateFormat.getDateTimeInstance(DateFormat.MEDIUM, DateFormat.SHORT).format(new java.util.Date(nextCheck));
				nextAlarmStr = getResources().getString(R.string.next_report_on) + ": " + nextAlarmStr;
				mPrefHolder.setSummary(nextAlarmStr);
			}
		} else {
			mPrefHolder = prefSet.findPreference(PREF_LAST_REPORT_ON);
			PreferenceCategory prefCat = (PreferenceCategory) prefSet.findPreference("pref_stats");
			prefCat.removePreference(mPrefHolder);
		}

		mPrefHolder = prefSet.findPreference(PREF_REPORT_INTERVAL);
		int tFrame = (int) Utilities.getTimeFrame();
		mPrefHolder.setSummary(getResources().getQuantityString(R.plurals.reporting_interval_days, tFrame, tFrame));

		// Cancel notification on app open, in case it doesn't AutoCancel
		NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		nm.cancel(Utilities.NOTIFICATION_ID);

		Utilities.checkIconVisibility(this);
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		if (preference == mViewStats) {
			// Display the stats page
			Uri uri = Uri.parse(Utilities.getStatsUrl() + "stats.php");
			startActivity(new Intent(Intent.ACTION_VIEW, uri));
		} else {
			// If we didn't handle it, let preferences handle it.
			return super.onPreferenceTreeClick(preferenceScreen, preference);
		}
		return true;
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue) {
		return false;
	}

	@Override
	public void onDismiss(DialogInterface dialog) {
		if (!mOkClicked) {
			mEnableReporting.setChecked(false);
		}
	}


	  public void onClick(DialogInterface dialog, int which) {
 	 	if (which == DialogInterface.BUTTON_POSITIVE) {
	 		 mOkClicked = true;
	 		 mPrefs.edit().putBoolean(Const.ANONYMOUS_OPT_IN, true).apply();

	 		 mPersistentOptout.setChecked(false);
	 		 try {
	 			 File sdCard = Environment.getExternalStorageDirectory();
	 			 File dir = new File (sdCard.getAbsolutePath() + "/.ROMStats");
	 			 File cookieFile = new File(dir, "optout");
	 			 cookieFile.delete();
	 			 Log.d(Const.TAG, "Persistent Opt-Out cookie removed successfully");
	 		 } catch (Exception e) {
	 			 Log.w(Const.TAG, "Unable to write persistent optout cookie", e);
	 		 }

	 		 ReportingServiceManager.launchService(this);
	 	 } else if (which == DialogInterface.BUTTON_NEGATIVE) {
	 		 mEnableReporting.setChecked(false);
	 	 } else {
	 		 Uri uri = Uri.parse("http://www.cyanogenmod.com/blog/cmstats-what-it-is-and-why-you-should-opt-in");
	 		 startActivity(new Intent(Intent.ACTION_VIEW, uri));
	 	 }
	  }

		@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.menu, menu);

		// remove Uninstall option if RomStats is installed as System App
		try {
			PackageManager pm = getPackageManager();
			ApplicationInfo appInfo = pm.getApplicationInfo(getPackageName(), 0);

			// Log.d(Utilities.TAG, "App is installed in: " +
			// appInfo.sourceDir);
			// Log.d(Utilities.TAG, "App is system: " + (appInfo.flags &
			// ApplicationInfo.FLAG_SYSTEM));

			if ((appInfo.sourceDir.startsWith("/data/app/"))
					&& (appInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
				// installed as user app, ok
			} else {
				menu.findItem(R.id.uninstall).setVisible(false);
			}
		} catch (Exception e) {
			menu.findItem(R.id.uninstall).setVisible(false);
		}

		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.learn_more:
			new AlertDialog.Builder(this)
				.setMessage(this.getResources().getString(R.string.anonymous_statistics_warning))
				.setTitle(R.string.anonymous_statistics_warning_title)
				.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						dialog.dismiss();
					}
				}).show();
			break;
		case R.id.uninstall:
			uninstallSelf();
			break;
		case R.id.hideicon:
			DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					switch (which) {
					case DialogInterface.BUTTON_POSITIVE:
						// Yes button clicked
						hideLauncherIcon();
						dialog.dismiss();
						break;

					case DialogInterface.BUTTON_NEGATIVE:
						// No button clicked
						dialog.dismiss();
						break;
					}
				}
			};

			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder
					.setTitle(R.string.app_name)
					.setIcon(R.drawable.ic_launcher)
					.setMessage(R.string.pref_hideicon_desc)
					.setPositiveButton(android.R.string.ok, dialogClickListener)
					.setNegativeButton(android.R.string.cancel, dialogClickListener)
					.setCancelable(true)
					.show();

			break;
		}

		return super.onOptionsItemSelected(item);
	}

	public void uninstallSelf() {
		Intent intent = new Intent(Intent.ACTION_DELETE);
		intent.setData(Uri.parse("package:" + getPackageName()));
		startActivity(intent);
	}

	public void hideLauncherIcon() {
		PackageManager p = getPackageManager();
		p.setComponentEnabledSetting(getComponentName(), PackageManager.COMPONENT_ENABLED_STATE_DISABLED, PackageManager.DONT_KILL_APP);

		try {
			File sdCard = Environment.getExternalStorageDirectory();
			File dir = new File (sdCard.getAbsolutePath() + "/.ROMStats");
			dir.mkdirs();
			File cookieFile = new File(dir, "hide_icon");

			FileOutputStream optOutCookie = new FileOutputStream(cookieFile);
			OutputStreamWriter oStream = new OutputStreamWriter(optOutCookie);
	        oStream.write("true");
	        oStream.close();
	        optOutCookie.close();
		} catch (IOException e) {
			Log.e(Const.TAG, "Error while writing 'hide_icon' file to sdcard", e);
		}
	}

}
