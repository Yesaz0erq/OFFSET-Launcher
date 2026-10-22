import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

/** Deliberate failure for testing the launcher's native report page. */
public class GuiCrashProbe {
    public static void main(String[] args) throws Exception {
        Files.createDirectories(Path.of("logs"));
        Files.writeString(Path.of("logs", "latest.log"),
            "GUI fixture: 游戏初始化失败\nThis is a deliberate test failure.\n",
            StandardCharsets.UTF_8);
        System.out.println("GUI fixture stdout: process started");
        System.err.println("GUI fixture stderr: 初始化失败");
        throw new IllegalStateException("OFFSET deliberate GUI crash probe");
    }
}
