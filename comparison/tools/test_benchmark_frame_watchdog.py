"""Real process timeout and disarm tests for the benchmark frame watchdog."""
import subprocess,tempfile,unittest
from pathlib import Path

class Watchdog(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();cls.root=Path(cls.tmp.name)
        src=cls.root/'watch.cpp';src.write_text('''#include "comparison/tools/frame_deadline.h"
int main(int argc,char**){FrameDeadline d(.05);if(argc>1){d.complete(FrameDeadline::Clock::now());
std::this_thread::sleep_for(std::chrono::milliseconds(100));return 0;}
std::this_thread::sleep_for(std::chrono::seconds(2));return 1;}''')
        cls.exe=cls.root/'watch'
        subprocess.run(['c++','-std=c++20','-pthread','-I',str(Path(__file__).resolve().parents[2]),str(src),'-o',str(cls.exe)],check=True,capture_output=True)
    @classmethod
    def tearDownClass(cls):cls.tmp.cleanup()
    def test_stalled_frame_terminates_process(self):
        r=subprocess.run([str(self.exe)],capture_output=True,timeout=1)
        self.assertEqual(r.returncode,124);self.assertIn(b'FRAME_DEADLINE_EXCEEDED',r.stderr)
    def test_completed_frame_disarms_watchdog(self):
        r=subprocess.run([str(self.exe),'completed'],capture_output=True,timeout=1)
        self.assertEqual(r.returncode,0)

if __name__=='__main__':unittest.main()
