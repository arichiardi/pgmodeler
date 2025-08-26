v2.0.0-alpha
------
*Release date: September 12, 2025*
*Changes since:* ***v1.2.x***

**Attention:** pgModeler 2.0.0-alpha introduces configuration changes that may affect compatibility with 1.x settings. On the first launch, pgModeler will automatically attempt to migrate your existing settings. Please note that this is an early development release where stability issues may occur. It is recommended that all models and databases be backed up before use. Please report any found bugs for prompt resolution. The mentioned migration process helps transition to improved configurations while minimizing disruption to your workflow.

**Summary:** After four months of work on the first development iteration of the next major release, I proudly present to you pgModeler 2.0.0-alpha. The goal of this release was exclusively to improve the overall user experience by adjusting several UI components in the hope of making the usage more comfortable and pleasant. Of course, this version does not include all intended changes, as they will be slowly introduced during the development process of 2.0 until the stable release is reached. But great things are coming to pgModeler, not only UI changes but also some cool features that have been awaited for a long time! Below are some of the detailed changes in this version.

**Build system changes:** Since its early days, pgModeler has relied on qmake, Qt's build system, for the compilation process. Due to some particularities of the pgModeler project itself and some limitations in qmake, it was often necessary to create workarounds to facilitate the compilation of the tool on all three main OSes. This was increasing the complexity of the build scripts, making them difficult to maintain. So, the first step taken when starting the pgModeler 2.0 development was the complete migration from qmake to CMake. CMake is another build system widely used across the open source world, with a great feature set that makes it the right choice for projects that are constantly growing, like pgModeler. This migration to CMake is not backward compatible with qmake, which means that pgModeler can no longer be compiled using qmake.

**Major UI change:** As in the previous major release (1.x), this one brings a refreshed UI. Despite still being a work-in-progress that will be finished by the stable 2.0 release, this first iteration of UI adjustments includes a small redesign of the project logo and UI icon colors. The idea is to create something more flat and colorful. Another change in this alpha release is the integration of the configuration form, as well as the tools' forms (diff, import, export, and fix), into the main window for a more fluid experience, reducing the number of modal dialogs in the tool, which could be very frustrating in several usage scenarios. The next UI adjustments will include the redesign of model object editing forms and many other elements.

**Improved diff tool:**

**Improved import tool:**

**Improved export tool:**

**Improved fix tools:**




