import java.io.FileOutputStream
import java.io.ByteArrayOutputStream
import java.util.Date
import java.text.SimpleDateFormat

plugins {
    `cpp-library`
    `cpp-unit-test`
}

repositories {
    maven {
        url = uri("https://repo.gradle.org/gradle/libs-snapshots-local/")
    }
}

unitTest {
    dependencies {
        implementation("org.gradle.cpp-samples:googletest:1.9.0-gr4-SNAPSHOT")
    }
    binaries.configureEach(CppTestExecutable::class.java) {
        if (toolChain is Gcc && targetMachine.operatingSystemFamily.isLinux) {
            linkTask.get().linkerArgs.add("-lpthread")
        }

        if (targetMachine.operatingSystemFamily.isWindows) {
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/user32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/gdi32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/advapi32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/shell32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/comdlg32.lib")
        }
    }
}

// Set the target operating system and architecture for this application
library {
    //add sources and macros for ultimate or ce version
    when (properties["facet"]) {
        "ultimate" -> {
            source {
                from("src/main/cpp")
                from("src/main/LPE-CS/cpp")
            }
            publicHeaders {
                from("src/main/headers")
            }
            privateHeaders {
                from("src/main/LPE-CS/headers")
            }
        }
    }

    binaries.configureEach(CppSharedLibrary::class.java) {
        //macros for ultimate or ce version
        when (properties["facet"]) {
            "ultimate" -> compileTask.get().macros["LPE_ULTIMATE"] = null
        }

        //add build details to macros
        compileTask.get().macros["BUILD_VERSION"] = "\"${getVersionNameFromGit()}\""
        compileTask.get().macros["BUILD_DATE"] = "\"${getBuildDate()}\""

        // Define toolchain-specific compiler options
        when (toolChain) {
            is Gcc, is Clang -> compileTask.get().compilerArgs.addAll(listOf("-std=c++17", "-fpermissive"))
            is VisualCpp -> compileTask.get().compilerArgs.addAll(listOf("/std:c++17", "/permissive-"))
        }

        if (targetMachine.operatingSystemFamily.isWindows) {
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/user32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/gdi32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/advapi32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/shell32.lib")
            linkTask.get().lib("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/comdlg32.lib")

            val dll = file(System.getProperty("user.home") + "/AppData/Roaming/REAPER/UserPlugins/reaper_livepresets.dll")
            linkTask.get().linkedFile.set(dll)

            val cResTask = tasks.register("cRes ${name.capitalize()}", WindowsResourceCompile::class.java) {
                targetPlatform.set(compileTask.get().targetPlatform)
                toolChain.set(compileTask.get().toolChain)
                source.from("src/main/headers/resources/resource.rc")
                includes("C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/um")
                includes("C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/shared")
                includes("C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt")
                includes("src/main/headers")
                compilerArgs.addAll(toolChain.map {
                    when (it) {
                        is VisualCpp -> listOf(
                                "/v"
                        )
                        else -> listOf()
                    }
                })
                outputDir = layout.buildDirectory.dir("res/${compileTask.get().name}").get().asFile
            }

            val cRes = fileTree(cResTask.get().outputDir) {
                include("**/*.res")
            }
            linkTask.get().source(cRes)
            tasks["build"].dependsOn(cResTask)
        }

        if (targetMachine.operatingSystemFamily.isLinux) {
            linkTask.get().linkedFile.set(file("/home/frederik/.config/REAPER/UserPlugins/reaper_livepresets.so"))
            compileTask.get().macros["SWELL_TARGET_GDK"] = "3"
            compileTask.get().macros["NOMINMAX"] = null
            compileTask.get().macros["SWELL_PROVIDED_BY_APP"] = null
        }

        if (targetMachine.operatingSystemFamily.isMacOs) {
            linkTask.get().linkedFile.set(file("/Users/frederik/Library/Application Support/REAPER/UserPlugins/reaper_livepresets.dylibs"))
            compileTask.get().macros["NOMINMAX"] = null
            compileTask.get().macros["SWELL_PROVIDED_BY_APP"] = null
        }

        if (targetMachine.operatingSystemFamily.isLinux || targetMachine.operatingSystemFamily.isMacOs) {

            val genResTask = tasks.register("genRes${name.capitalize()}", Exec::class.java) {
                //setWorkingDir("$projectDir/WDL-library/main/cpp/WDL/swell/")
                executable = "php"
                val scriptPath = "$projectDir/WDL-library/src/main/cpp/WDL/swell/mac_resgen.php"
                val rcPath = "$projectDir/src/main/headers/resources/resource.rc"
                args(scriptPath, rcPath)

                doLast {
                    ant.withGroovyBuilder {
                        val srcPath = "$projectDir/src/main/headers/resources/resource.rc_mac_"
                        "move"("file" to "${srcPath}dlg", "todir" to "${buildDir}/res")
                        "move"("file" to "${srcPath}menu", "todir" to "${buildDir}/res")
                    }
                }
            }

            val cRes = fileTree("$buildDir/res/")
            compileTask.get().includes("$buildDir/res/")
            compileTask.get().dependsOn(genResTask)
        }

        compileTask.get().isDebuggable = true
        compileTask.get().isPositionIndependentCode = true
    }

    targetMachines.add(machines.linux.x86_64)
    targetMachines.add(machines.windows.x86_64)
    targetMachines.add(machines.macOS.x86_64)

    dependencies {
        implementation("cockos:WDL:1.0")
    }
}

fun getVersionNameFromGit() : String {
    return try {
        val stdout = ByteArrayOutputStream()
        exec {
            commandLine("git", "describe", "--tags", "--dirty")
            standardOutput = stdout
        }
        stdout.toString().trim()
    }
    catch (ignored: Exception) {
        ""
    }
}

fun getBuildDate() : String {
    val df = SimpleDateFormat("yyyy-MM-dd")
    return df.format(Date())
}