ROM Stats
=========

**Description**: Usage statistics for ROM developer (a la Cyanogenmod Stats)

CyanogenMod has a feature to report anonymous Statistics to the Cyanogenmod Team: on first boot the user is given the choice to send anonymous statistics to CyanogenMod website (http://stats.cyanogenmod.com).

I went ahead and got the source code of the CyanogenMod GitHub, made some modifications to make it run on any Rom (Android > 2.3.3).

By default, when the user aggree to send anonymous statistics, the app will report to the server once every 7 days (or the number of days defined in the parameter tframe, check below).

### Usage:
To use this in a ROM, it should contain the APK as either a User or a System application, and then add these lines in the build.prop in the System directory:

	# ROM Statistics and ROM Identification
	ro.romstats.url=http://www.[domainname].com/[subfolder]/
	ro.romstats.name=[The desired ROM name]
	ro.romstats.version=[ROM Version]
	ro.romstats.tframe=7

Then there needs to be a web application on the server as the domain (the URL parameter) where the data is submitted. For that now I wrote a simple php page with a mysql database to save the data. I will also post the code for this little web app.

The submitted data contains:
* **Device hash**: which is a salted MD5 hash of the IMEI (or wifi MAC, if imei is unavailable for some reason)
* **Device name**: the property "ro.product.model" of build.prop
* **Device version**: the property "ro.build.display.id" of build.prop
* **Country**: from the Android API, getNetworkCountryIso
* **Carrier**: from the Android API, getNetworkOperatorName
* **Carier ID**: from the Android API, getNetworkOperator
* **ROM Name**: from the newly added property "ro.romstats.name"
* **ROM version**: from the newly added property "ro.romstats.version"

in addition to this data, the database has an extra 2 columns:
* **First registration date**: the first time the device registered on the server
* **Last check-in date**: the last time the device (with same device hash) checked in, to remove inactive devices after X days

### Updates:
**Update 1 (20/01/2013)**:
I added a new option in the main screen, Uninstall, which will appear the app is installed as a User app, which allows the end user to uninstall the stats app completely from the deivce, and not just opt-out of stat submitting.

**Update 2 (02/02/2013)**:
A new parameter (tframe) has been added, to allow the developer decide on the frequency of reporting to the server, by default it reports once every 7 days, but with this parameter it can be customized.
