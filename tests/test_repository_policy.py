"""Exercise the safeguards used before public pushes and in CI."""

import importlib.util
from pathlib import Path
import unittest

script = Path(__file__).resolve().parents[1] / "scripts/check_repository.py"
spec = importlib.util.spec_from_file_location("policy", script)
policy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(policy)


class RepositoryPolicyTest(unittest.TestCase):
    def test_original_source_is_allowed(self):
        self.assertIsNone(policy.violation("src/main.cpp", b"int main() {}"))

    def test_game_data_is_rejected_case_insensitively(self):
        for name in ["SCPH1001.BIN", "game.cue", "disc.7z", "music.wav", "texture.TIM"]:
            with self.subTest(name=name):
                self.assertIsNotNone(policy.violation(name, b"data"))

    def test_renamed_private_input_is_rejected(self):
        self.assertIsNotNone(policy.violation("local/game.txt", b"data"))
        self.assertIsNotNone(policy.violation("docs/innocent.txt", b"binary\0data"))

    def test_modified_tool_binary_is_rejected(self):
        self.assertIsNotNone(policy.violation(policy.WRAPPER, b"different contents"))


if __name__ == "__main__":
    unittest.main()
