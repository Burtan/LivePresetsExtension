plugins {
    `cpp-library`
}
group = "cockos"
version = "1.0"

tasks.withType(CreateStaticLibrary::class).configureEach {
    val cObjs = project.fileTree("build/objs") {
        include("**/*.obj")
    }
    source(cObjs)
}

library {
    binaries.configureEach(CppStaticLibrary::class.java) {
        compileTask.get().isPositionIndependentCode = true

        // Define toolchain-specific compiler options
        when (toolChain) {
            is Gcc, is Clang -> compileTask.get().compilerArgs.addAll(listOf("-std=c++14"))
            is VisualCpp -> compileTask.get().compilerArgs.add("/std:c++14")
        }

        if (targetMachine.operatingSystemFamily.isLinux) {
            //swell
            compileTask.get().source.from("src/main/cpp/WDL/swell/swell-modstub-generic.cpp")

            //Add macros to use swell from reaper (reduces size)
            compileTask.get().macros["SWELL_TARGET_GDK"] = "3"
            compileTask.get().macros["NOMINMAX"] = null
            compileTask.get().macros["SWELL_PROVIDED_BY_APP"] = null
        }

        if (targetMachine.operatingSystemFamily.isMacOs) {
            //swell
            compileTask.get().source.from("src/main/cpp/WDL/swell/swell.cpp")
            compileTask.get().source.from("src/main/cpp/WDL/swell/swell-ini.cpp")

            compileTask.get().macros["NOMINMAX"] = null
        }
    }

    binaries.configureEach {
        val cCompileTask = tasks.register("compile" + name.capitalize() + "C", CCompile::class) {
            toolChain.set(compileTask.map { it.toolChain.get() })
            // Take configuration from C++ compile tasks
            targetPlatform.set(compileTask.map { it.targetPlatform.get() })
            includes.from(compileTask.map { it.includes })
            systemIncludes.from(compileTask.map { it.systemIncludes })

            // Configure the C source file location
            when (toolChain.get()) {
                is VisualCpp -> {
                    source.from("src/main/cpp/WDL/win32_utf8.c")
                }
            }
            source.from("src/main/cpp/WDL/zlib/adler32.c")
            source.from("src/main/cpp/WDL/zlib/crc32.c")
            source.from("src/main/cpp/WDL/zlib/inffast.c")
            source.from("src/main/cpp/WDL/zlib/inflate.c")
            source.from("src/main/cpp/WDL/zlib/inftrees.c")
            source.from("src/main/cpp/WDL/zlib/zutil.c")

            // Must use another directory for proper up-to-date check
            objectFileDir.set(project.layout.buildDirectory.dir("objs/mainC"))

            // Unfortunately, this doesn't use the Provider API yet. The impact is minimized by using the lazy task API
            macros = compileTask.get().macros
            isOptimized = compileTask.get().isOptimized
            isDebuggable = compileTask.get().isDebuggable
            isPositionIndependentCode = compileTask.get().isPositionIndependentCode
        }

        val objcCompileTask = tasks.register("compile" + name.capitalize() + "objC", ObjectiveCppCompile::class) {
            toolChain.set(compileTask.map { it.toolChain.get() })
            // Take configuration from C++ compile tasks
            targetPlatform.set(compileTask.map { it.targetPlatform.get() })
            includes.from(compileTask.map { it.includes })
            systemIncludes.from(compileTask.map { it.systemIncludes })

            // Configure the objC source file location
            source.from("src/main/cpp/WDL/swell/swell-miscdlg.mm")
            source.from("src/main/cpp/WDL/swell/swell-gdi.mm")
            source.from("src/main/cpp/WDL/swell/swell-kb.mm")
            source.from("src/main/cpp/WDL/swell/swell-menu.mm")
            source.from("src/main/cpp/WDL/swell/swell-misc.mm")
            source.from("src/main/cpp/WDL/swell/swell-dlg.mm")
            source.from("src/main/cpp/WDL/swell/swell-wnd.mm")

            // Must use another directory for proper up-to-date check
            objectFileDir.set(project.layout.buildDirectory.dir("objs/mainObjC"))

            // Unfortunately, this doesn't use the Provider API yet. The impact is minimized by using the lazy task API
            macros = compileTask.get().macros
            isOptimized = compileTask.get().isOptimized
            isDebuggable = compileTask.get().isDebuggable
            isPositionIndependentCode = compileTask.get().isPositionIndependentCode
        }

        tasks.filter { it.name.contains("createDebug") }.forEach {
            it.dependsOn(cCompileTask)
            if (targetMachine.operatingSystemFamily.isMacOs) {
                it.dependsOn(objcCompileTask)
            }
        }

    }

    linkage.set(listOf(Linkage.STATIC))
    source {
        //jnetlib
        from("src/main/cpp/WDL/jnetlib/httpget.cpp")
        from("src/main/cpp/WDL/jnetlib/util.cpp")
        from("src/main/cpp/WDL/jnetlib/connection.cpp")
        from("src/main/cpp/WDL/jnetlib/asyncdns.cpp")

        //libpng
        from("src/main/cpp/WDL/libpng/png.cpp")
        from("src/main/cpp/WDL/libpng/pngerror.cpp")
        from("src/main/cpp/WDL/libpng/pngget.cpp")
        from("src/main/cpp/WDL/libpng/pngmem.cpp")
        from("src/main/cpp/WDL/libpng/pngpread.cpp")
        from("src/main/cpp/WDL/libpng/pngread.cpp")
        from("src/main/cpp/WDL/libpng/pngrio.cpp")
        from("src/main/cpp/WDL/libpng/pngrtran.cpp")
        from("src/main/cpp/WDL/libpng/pngrutil.cpp")
        from("src/main/cpp/WDL/libpng/pngset.cpp")
        from("src/main/cpp/WDL/libpng/pngtrans.cpp")

        //lice
        from("src/main/cpp/WDL/lice/lice.cpp")

        //wdl
        from("src/main/cpp/WDL/wingui/wndsize.cpp")
        from("src/main/cpp/WDL/wingui/scrollbar/coolscroll.cpp")
    }
    publicHeaders {
        from("src/main/cpp")
    }

    targetMachines.add(machines.linux.x86_64)
    targetMachines.add(machines.windows.x86_64)
    targetMachines.add(machines.windows.x86)
    targetMachines.add(machines.macOS.x86_64)
}