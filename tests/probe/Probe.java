import java.nio.file.*;
import java.nio.charset.StandardCharsets;
public class Probe {
 public static void main(String[] args) throws Exception {
  Files.writeString(Path.of("probe-result.txt"),String.join("\n",args),StandardCharsets.UTF_8);
 }
}
