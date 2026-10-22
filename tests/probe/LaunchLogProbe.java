import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

/** Actual child process: output, split credentials, fresh and stale game reports. */
public class LaunchLogProbe {
    public static void main(String[] args) throws Exception {
        Map<String,String> a = new HashMap<>();
        for (int i=0;i<args.length;i+=2) a.put(args[i],args[i+1]);
        String mode=a.get("--mode"), secret=a.get("--token");
        String key="fixture-curse-key-0123456789-abcdef";
        // Exercise the JVM's default PrintStream encoding, not explicit UTF-8 bytes.
        System.out.println("stdout unicode 标准输出 中文 日本語");
        System.err.println("stderr captured 初始化失败 中文 日本語");
        if (!"UTF-8".equals(System.getProperty("stdout.encoding"))
                || !"UTF-8".equals(System.getProperty("stderr.encoding")))
            throw new AssertionError("Captured stream encoding was not selected");
        // Metadata explicitly chooses COMPAT. The launcher must preserve that
        // independent file-I/O choice while selecting the standard streams.
        if (!Objects.equals(System.getProperty("file.encoding"),System.getProperty("native.encoding")))
            throw new AssertionError("file.encoding was changed by output capture");
        System.out.println("default file charset preserved: "+System.getProperty("file.encoding"));
        out("split="+secret.substring(0,21));
        System.out.flush(); Thread.sleep(70);
        out(secret.substring(21)+"\nkey="+key+"\n");
        if(mode.equals("spam")) {
            byte[] block=("x".repeat(4032)+secret+"\n").getBytes(StandardCharsets.UTF_8);
            for(int i=0;i<1500;i++) System.out.write(block);
            return;
        }
        if(mode.equals("crash")) {
            Files.createDirectories(Path.of("logs"));
            Files.createDirectories(Path.of("crash-reports"));
            Files.writeString(Path.of("logs/latest.log"),"fresh-game-log "+secret+"\n",StandardCharsets.UTF_8);
            Files.writeString(Path.of("crash-reports/crash-current.txt"),"fresh-crash-report "+key+"\n",StandardCharsets.UTF_8);
            Files.writeString(Path.of("hs_err_pid-fixture.log"),"fresh-jvm-report "+secret+"\n",StandardCharsets.UTF_8);
            System.exit(23);
        }
    }
    private static void out(String s) { System.out.print(s); }
}
