import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LinkedHashMap;
import java.util.Map;

/** An actual JVM process checks the launcher's per-profile filesystem contract. */
public class ProfileProbe {
    public static void main(String[] args) throws Exception {
        Map<String, String> values = new LinkedHashMap<>();
        for (int i = 0; i < args.length; i += 2) values.put(args[i], args[i + 1]);
        String profile = values.get("--profile");
        Path root = Path.of(System.getenv("OFFSET_TEST_GAME")).toRealPath();
        boolean isolated = profile.equals("probe-isolated");
        Path expectedData = isolated ? root.resolve("versions").resolve(profile).resolve("game") : root;
        Path working = Path.of("").toRealPath();
        require(working.equals(expectedData.toRealPath()), "working directory");
        require(Path.of(values.get("--gameDir")).toRealPath().equals(working), "game directory");
        require(Path.of(values.get("--assetsDir")).toRealPath().equals(root.resolve("assets")), "assets path");
        require(Path.of(System.getProperty("probe.libraries")).toRealPath().equals(root.resolve("libraries")), "library path");
        require(Path.of(System.getProperty("java.library.path")).toRealPath().equals(root.resolve("versions").resolve(profile).resolve("offset-natives")), "native path");
        require(ProfileLibrary.value().equals("root-cache-library"), "library class loading");
        Path library = Path.of(ProfileLibrary.class.getProtectionDomain().getCodeSource().getLocation().toURI()).toRealPath();
        require(library.equals(root.resolve("libraries/test/profile/library/1/library-1.jar")), "shared library cache");
        require(Files.readString(Path.of(values.get("--assetsDir")).resolve("profile-asset.txt")).equals("shared-root-asset"), "asset file");
        long expectedMemory = Long.parseLong(values.get("--memory")) * 1024 * 1024;
        long maximum = Runtime.getRuntime().maxMemory();
        require(maximum <= expectedMemory && maximum >= expectedMemory - 32 * 1024 * 1024, "memory override");
        Files.writeString(working.resolve("result-" + profile + ".txt"),
            "profile=" + profile + "\nworking=" + working + "\nmaxMemory=" + maximum
            + "\nlibrary=" + library + "\nassets=" + values.get("--assetsDir") + "\n",
            StandardCharsets.UTF_8);
    }

    private static void require(boolean condition, String label) {
        if (!condition) throw new IllegalStateException("Failed: " + label);
    }
}
