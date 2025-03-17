import os
from pathlib import Path
from cutekit import cli, shell, const


WPT_URL = "https://github.com/vaev-org/wpt"
WPT_ROOT = Path(const.PROJECT_CK_DIR) / "wpt"


def _ensureWpts():
    if not WPT_ROOT.exists():
        try:
            print(f"Cloning WPTs from {WPT_URL}...")
            shell.exec("git", "clone", WPT_URL, str(WPT_ROOT), "--depth=1")
        except Exception as e:
            if WPT_ROOT.exists():
                WPT_ROOT.rmdir()
            raise RuntimeError(f"Failed to clone WPTs: {e}")


@cli.command(None, "wpt", "Manage the WPTs")
def _(): ...


class WptArgs:
    args: list[str] = cli.extra("args", "Args to pass to the WPTs")


@cli.command("a", "wpt/exec", "Exec ./wpt")
def _(args: WptArgs):
    cmd = [
        "./wpt",
    ] + args.args

    # Run the WPTs
    shell.exec(
        *cmd,
        cwd=str(WPT_ROOT),
    )


@cli.command("a", "wpt/run", "Run the WPTs")
def _(args: WptArgs):
    _ensureWpts()

    cmd = [
        "./wpt",
        "run",
        "paper_muncher",
        "--webdriver-binary",
        "paper_muncher_webdriver",
        "--test-type=reftest",
        # "--log-wptreport ${{ runner.temp }}/wpt_report_${{ matrix.current-chunk }}.json",
        "--no-fail-on-unexpected",
        "--include-file",
        str(Path("meta/wpt/whitelist").absolute()),
    ] + args.args

    # Run the WPTs
    shell.exec(
        *cmd,
        cwd=str(WPT_ROOT),
    )
