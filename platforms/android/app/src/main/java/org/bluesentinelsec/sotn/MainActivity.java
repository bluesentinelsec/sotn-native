package org.bluesentinelsec.sotn;

import org.libsdl.app.SDLActivity;

public final class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        // libmain contains SDL3's implementation and JNI entrypoints.
        return new String[] { "main" };
    }

    @Override
    protected String[] getArguments() {
        return getIntent().getBooleanExtra("smoke_test", false)
            ? new String[] { "--smoke-test" } : new String[0];
    }
}
