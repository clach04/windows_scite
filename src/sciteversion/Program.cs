/*
 * Created by SharpDevelop.
 * User: Troy Simpson
 * Date: 17/10/2016
 * Time: 3:33 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.IO;
using System.Diagnostics;

namespace sciteversion
{
	class Program
	{
		public static void Main(string[] args)
		{
	     	string s = args[0];

	     	try {
		     	FileVersionInfo ver = FileVersionInfo.GetVersionInfo(s);
				string retver = ver.FileVersion.Split('.')[0] + "." + ver.FileVersion.Split('.')[1] + "." + ver.FileVersion.Split('.')[2];
				
				string xmlcontent = 
	@"<Project xmlns=""http://schemas.microsoft.com/developer/msbuild/2003"">
	  <PropertyGroup>
		<sciteversion>" + retver + @"</sciteversion>
	  </PropertyGroup>
	</Project>
	";
				File.WriteAllText("..\\..\\..\\scite-win32\\Setup.Version.Proj", xmlcontent);
				//File.WriteAllText("..\\..\\..\\scite-win32\\scite-win32\\Setup.Version.Proj", xmlcontent);
	     	} catch (Exception ex) {
	     		Console.WriteLine("Error: " + ex.Message);
			}
		}
	}
}