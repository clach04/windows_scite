
This is a SNAPSHOT of scite-5.4.1 (circa Dec 26 2023) with Microsoft Windows installer and Context Menu for Windows Explorer.


  * [SciTE](https://scintilla.org/SciTE.html), or SCIntilla based Text Editor, is a cross-platform text editor created by Neil Hodgson.
  * Context Menu Extension for SciTE (Win32) wscitecm.dll - Context Menu for Windows Explorer originally created Andre Burgaud
      * Andre Burgaud https://www.burgaud.com/scite-context-menu/
        TODO readme from website and apply to md, e.g. link to msvcrt DLLs
        https://github.com/andreburgaud/wscitecm
      * Angelo Mandato https://www.spaceblue.com/products/context-menu-for-scite/
  * Installer
      * Bruce Dodson https://gisdeveloper.tripod.com/scite.html - for install
        differences to upstream:
          * bitlib lua lib
          * bunch of whitespace/indentation lua code functions
          * external binaries asytle/tidy/somethingelse and menu items
          * API help
          * bunch of config changes
              * nice status line(s)
              * default file *.txt?
      * This project was created by Troy Simpson
          * shchange by Troy Simpson, sends a shell change notification so that after an install the wscitecm context menu will show up
          * wscitecm in this project is modified to have 2 menu entries


The original website is no longer available. https://web.archive.org/web/20240413060226/http://www.ebswift.com/scite-text-editor-installer.html
See mhtml export in https://github.com/clach04/windows_scite/releases/tag/v5.4.1

Some config/property files differ from the upstream ones for SciTE.

Downloads in https://github.com/clach04/windows_scite/releases/
