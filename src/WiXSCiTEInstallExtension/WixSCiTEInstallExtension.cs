/*
 * Created by SharpDevelop.
 * User: Troy
 * Date: 27/06/2015
 * Time: 11:30 AM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 * 
 * This custom extension gets the file version info we can use to partially automate our install.
 */
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using Microsoft.Tools.WindowsInstallerXml;
using System.Diagnostics;

namespace WiXSCiTEInstallExtension
{
	/// <summary>
	/// Description of UserControl1.
	/// </summary>
	partial class WiXSCiTEInstallExtension : WixExtension
	{
		// wix stuff
		private ebswiftProcessorExtension preprocessorExtension;
		
		public WiXSCiTEInstallExtension()
		{
			//
			// The InitializeComponent() call is required for Windows Forms designer support.
			//
			InitializeComponent();
			
			//
			// TODO: Add constructor code after the InitializeComponent() call.
			//
		}
		
		public override PreprocessorExtension PreprocessorExtension {
     		get
			{ 
				if (this.preprocessorExtension == null) 
				{ 
				 this.preprocessorExtension = new ebswiftProcessorExtension();
				}
				
				return this.preprocessorExtension; 
			} 
		}
	}

	public class ebswiftProcessorExtension: PreprocessorExtension 
	{
		private static string[] prefixes = { "ebswift" }; 
		public override string[] Prefixes { get { return prefixes; } }

		public override string GetVariableValue(string prefix, string name) 
		{ 
		  string result = null; 
		 // Based on the namespace and name, define the resulting string. 
		 switch (prefix) 
		 { 
		     case "ebswift": 
		         switch (name) 
		         { 
		             case "xxxx": 
		                // This could be looked up from anywhere you can access from your code.
		                result = "replaced"; 
		                break; 
		         } 
		         break; 
		 }  
		 return result; 
		}  
		
		public override string EvaluateFunction(string prefix, string function, string[] args) 
		{ 
		 string result = null; 
		 switch (prefix) 
		 { 
		     case "ebswift": 
		         switch (function)  
		         { 
		             case "sciteversion": 
		                 if (0 < args.Length)  
		                 { 
		                 	string s = args[0];
		                 	FileVersionInfo ver = FileVersionInfo.GetVersionInfo(s);
		                 	
							string retver = ver.FileVersion.Split('.')[0] + "." + ver.FileVersion.Split('.')[1] + "." + ver.FileVersion.Split('.')[2];
		                    result = retver; 
		                 } 
		                 else 
		                 { 
		                    result = string.Empty;  
		                 } 
		                 break;  
		         }  
		         break;  
		 }
		 return result; 
		}
	}
}