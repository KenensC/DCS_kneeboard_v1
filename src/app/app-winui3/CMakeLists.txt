set(VERSION_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
create_version_rc(
  OpenKneeboard-App-WinUI3_version-rc
  OpenKneeboard-App-WinUI3
  OpenKneeboardApp.exe
  ${VERSION_RC_FILE}
)

set(
  CMAKE_CXX_STANDARD_LIBRARIES
  # Keep stuff that NuGet package references added
  "%(AdditionalDependencies)"
)

add_executable(
  OpenKneeboard-App-WinUI3
  WIN32
  Resource.rc ${VERSION_RC_FILE} app.manifest
  HelpPage.idl HelpPage.xaml HelpPage.xaml.cpp HelpPage.xaml.h
  AdvancedSettingsPage.idl AdvancedSettingsPage.xaml AdvancedSettingsPage.xaml.cpp AdvancedSettingsPage.xaml.h
  App.idl App.xaml App.xaml.cpp App.xaml.h
  CheckDCSHooks.cpp CheckDCSHooks.h
  CheckForUpdates.cpp CheckForUpdates.h
  CheckRuntimeFiles.cpp CheckRuntimeFiles.h
  DegreesValueConverter.cpp DegreesValueConverter.h DegreesValueConverter.idl
  ExecutableIconFactory.cpp ExecutableIconFactory.h
  GameSettingsPage.idl GameSettingsPage.xaml GameSettingsPage.xaml.cpp GameSettingsPage.xaml.h
  Globals.cpp Globals.h
  InputBindingsControl.idl InputBindingsControl.xaml InputBindingsControl.xaml.cpp InputBindingsControl.xaml.h
  InputDeviceUIData.cpp InputDeviceUIData.h
  InputSettingsPage.idl InputSettingsPage.xaml InputSettingsPage.xaml.cpp InputSettingsPage.xaml.h
  MainWindow.idl MainWindow.xaml MainWindow.xaml.cpp MainWindow.xaml.h
  MetersNumberFormatter.cpp MetersNumberFormatter.h MetersNumberFormatter.idl
  MultiplierValueConverter.cpp MultiplierValueConverter.h MultiplierValueConverter.idl
  NonVRSettingsPage.idl NonVRSettingsPage.xaml NonVRSettingsPage.xaml.cpp NonVRSettingsPage.xaml.h
  PercentValueConverter.cpp PercentValueConverter.h PercentValueConverter.idl
  ProcessListPage.idl ProcessListPage.xaml ProcessListPage.xaml.cpp ProcessListPage.xaml.h
  SettingsPage.idl SettingsPage.xaml SettingsPage.xaml.cpp SettingsPage.xaml.h
  TabPage.idl TabPage.xaml TabPage.xaml.cpp TabPage.xaml.h
  TabSettingsPage.idl TabSettingsPage.xaml TabSettingsPage.xaml.cpp TabSettingsPage.xaml.h
  VRSettingsPage.idl VRSettingsPage.xaml VRSettingsPage.xaml.cpp VRSettingsPage.xaml.h
)

include(winui3-boilerplate.cmake)

set_property(
  SOURCE App.xaml
  PROPERTY VS_XAML_TYPE
  "ApplicationDefinition"
)

set_property(
  SOURCE InputDeviceUIData.h InputDeviceUIData.cpp
  PROPERTY VS_SETTINGS
  "DependentUpon=InputDeviceSettingsPage.idl"
)

target_link_libraries(
  OpenKneeboard-App-WinUI3
  PRIVATE
  OpenKneeboard-App-Common
  OpenKneeboard-FilesDiffer
  OpenKneeboard-GetMainHWND
  OpenKneeboard-RuntimeFiles
  OpenKneeboard-config
  OpenKneeboard-dprint
  OpenKneeboard-games
  OpenKneeboard-version
  System::Dbghelp
  System::Shell32
  System::User32
)

target_include_directories(
  OpenKneeboard-App-WinUI3
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(
  OpenKneeboard-App-WinUI3
  PRIVATE
  DISABLE_XAML_GENERATED_MAIN=1
)

target_compile_options(
  OpenKneeboard-App-WinUI3
  PRIVATE
  # We intentionally throw from noexcept functions to get more useful
  # backtraces/crash dumps from winrt event handlers
  "/wd4297"
)

set_property(
  TARGET OpenKneeboard-App-WinUI3
  PROPERTY OUTPUT_NAME OpenKneeboardApp # .exe
)

install(
  DIRECTORY
  # Trailing / copies the contents instead of the directory itself,
  # i.e. avoids installing `bin/Debug/`
  "$<TARGET_FILE_DIR:OpenKneeboard-App-WinUI3>/"
  TYPE BIN
  PATTERN "*.pdb" EXCLUDE
  PATTERN "*.lib" EXCLUDE
)
install(
  FILES
  "$<TARGET_PDB_FILE:OpenKneeboard-App-WinUI3>"
  TYPE BIN
  COMPONENT DebugSymbols
)

set_property(
  INSTALL
  "bin/OpenKneeboardApp.exe"
  PROPERTY
  CPACK_START_MENU_SHORTCUTS "OpenKneeboard"
)
set_property(
  INSTALL
  "bin/OpenKneeboardApp.exe"
  PROPERTY
  CPACK_DESKTOP_SHORTCUTS "OpenKneeboard"
)
