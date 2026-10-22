param([switch]$Run)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$fixtureDir = Join-Path $PSScriptRoot 'out\servers-net-fixture'
New-Item -ItemType Directory -Path $fixtureDir -Force | Out-Null
if (-not ('OffsetSlpFixtures' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
public sealed class OffsetSlpFixtures : IDisposable {
    public readonly Dictionary<string,int> Ports = new Dictionary<string,int>();
    public readonly List<string> Failures = new List<string>();
    private readonly List<TcpListener> listeners = new List<TcpListener>();
    private readonly List<TcpClient> clients = new List<TcpClient>();
    private volatile bool stopped;
    static byte[] Var(int value) {
        var bytes = new List<byte>();
        do { byte next = (byte)(value & 127); value >>= 7;
             if (value != 0) next |= 128; bytes.Add(next); } while (value != 0);
        return bytes.ToArray();
    }
    static int ReadVar(Stream stream) {
        int result = 0;
        for (int shift = 0; shift < 35; shift += 7) {
            int next = stream.ReadByte(); if (next < 0) throw new IOException("EOF");
            result |= (next & 127) << shift;
            if ((next & 128) == 0) return result;
        }
        throw new IOException("long varint");
    }
    static byte[] Exact(Stream stream, int bytes) {
        if (bytes < 0 || bytes > 2048) throw new IOException("bad request frame");
        byte[] data = new byte[bytes]; int at = 0;
        while (at < data.Length) { int got = stream.Read(data, at, data.Length-at);
            if (got == 0) throw new IOException("EOF"); at += got; }
        return data;
    }
    void Serve(TcpListener listener, string name, string expectedHost) {
        try {
            using (TcpClient client = listener.AcceptTcpClient()) {
                lock (clients) clients.Add(client);
                client.ReceiveTimeout = 7000; client.SendTimeout = 7000;
                var stream = client.GetStream();
                byte[] handshake = Exact(stream, ReadVar(stream));
                var memory = new MemoryStream(handshake);
                if (ReadVar(memory) != 0 || ReadVar(memory) != 47) throw new IOException("bad handshake");
                int hostBytes = ReadVar(memory);
                string host = Encoding.UTF8.GetString(Exact(memory, hostBytes));
                int port = (memory.ReadByte() << 8) | memory.ReadByte();
                if (host != expectedHost || port != Ports[name] || ReadVar(memory) != 1 || memory.Position != memory.Length)
                    throw new IOException("incorrect host, port or next state");
                byte[] request = Exact(stream, ReadVar(stream));
                if (request.Length != 1 || request[0] != 0) throw new IOException("bad status request");
                if (name == "timeout" || name == "cancel") { Thread.Sleep(6500); return; }
                if (name == "huge") { byte[] huge = Var(65537); stream.Write(huge,0,huge.Length); return; }
                if (name == "malformed") { byte[] malformed = {128,128,128,128,128,128}; stream.Write(malformed,0,malformed.Length); return; }
                string json = "{\"version\":{\"name\":\"fixture\",\"protocol\":47},\"players\":{\"online\":7,\"max\":25}}";
                if (name == "badcount") json = "{\"players\":{\"online\":\"7\",\"max\":25}}";
                if (name == "unknown") json = "{\"description\":\"status counts hidden\"}";
                byte[] text = Encoding.UTF8.GetBytes(json);
                var payload = new List<byte>(); payload.Add(0); payload.AddRange(Var(text.Length)); payload.AddRange(text);
                var frame = new List<byte>(); frame.AddRange(Var(payload.Count)); frame.AddRange(payload);
                byte[] response = frame.ToArray();
                if (name == "fragment") {
                    for (int i=0;i<response.Length;i++) { stream.Write(response,i,1); Thread.Sleep(2); }
                } else stream.Write(response,0,response.Length);
            }
        } catch (Exception error) {
            if (!stopped) lock(Failures) Failures.Add(name+": "+error.Message);
        }
    }
    void Start(string name, bool ipv6, string expectedHost) {
        var listener = new TcpListener(ipv6 ? IPAddress.IPv6Loopback : IPAddress.Loopback,0);
        listener.Start(); Ports[name] = ((IPEndPoint)listener.LocalEndpoint).Port;
        listeners.Add(listener);
        var thread = new Thread(() => Serve(listener,name,expectedHost)); thread.IsBackground = true; thread.Start();
    }
    public OffsetSlpFixtures() {
        Start("normal",false,"127.0.0.1"); Start("fragment",false,"localhost");
        Start("huge",false,"127.0.0.1"); Start("malformed",false,"127.0.0.1");
        Start("badcount",false,"127.0.0.1"); Start("unknown",false,"127.0.0.1");
        Start("timeout",false,"127.0.0.1"); Start("ipv6",true,"::1");
        Start("cancel",false,"127.0.0.1");
        Start("srv",false,"fixture-srv.test");
        var unused = new TcpListener(IPAddress.Loopback,0); unused.Start();
        Ports["offline"] = ((IPEndPoint)unused.LocalEndpoint).Port; unused.Stop();
    }
    public void Dispose() {
        stopped = true;
        foreach (var listener in listeners) listener.Stop();
        lock(clients) foreach(var client in clients) client.Close();
    }
}
'@
}
$fixtures = [OffsetSlpFixtures]::new()
$oldFixture = $env:OFFSET_SERVERS_NET_FIXTURE
try {
    $ini = Join-Path $fixtureDir 'ports.ini'
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add('[Fixture]')
    foreach ($pair in $fixtures.Ports.GetEnumerator()) { $lines.Add("$($pair.Key)=$($pair.Value)") }
    [IO.File]::WriteAllLines($ini,$lines,[Text.Encoding]::Unicode)
    if ($Run) {
        $env:OFFSET_SERVERS_NET_FIXTURE = $ini
        $savedInclude = $env:INCLUDE
        try {
            $env:INCLUDE = Join-Path $root 'tools\INCLUDE'
            $exe = Join-Path $PSScriptRoot 'out\bin\servers-net.exe'
            & (Join-Path $root 'tools\FASM.EXE') '-m' '262144' (Join-Path $PSScriptRoot 'servers-net.asm') $exe
            if ($LASTEXITCODE -ne 0) { throw 'Could not assemble servers-net.' }
            $start = [Diagnostics.ProcessStartInfo]::new()
            $start.FileName = $exe
            $start.WorkingDirectory = $root
            $start.UseShellExecute = $false
            $start.CreateNoWindow = $true
            $start.RedirectStandardOutput = $true
            $start.RedirectStandardError = $true
            $process = [Diagnostics.Process]::Start($start)
            if (!$process.WaitForExit(15000)) { $process.Kill(); throw 'SLP loopback test timed out.' }
            $output = $process.StandardOutput.ReadToEnd() + $process.StandardError.ReadToEnd()
            if ($process.ExitCode -ne 0) { throw "SLP loopback test failed at step $($process.ExitCode). $output" }
            if ($fixtures.Failures.Count -ne 0) { throw ($fixtures.Failures -join '; ') }
            Write-Host 'PASS servers-net: real IPv4/IPv6, hostname, SRV handshake, fragments, timeouts and invalid frames'
        } finally { $env:INCLUDE = $savedInclude }
    }
} finally {
    $env:OFFSET_SERVERS_NET_FIXTURE = $oldFixture
    $fixtures.Dispose()
}
