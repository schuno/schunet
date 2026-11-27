
#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys
import zipfile
from pathlib import Path


def get_git_ignored_files(directory: Path) -> set[str]:
    """Get ignored files using Git's own ignore engine."""

    try:
        result = subprocess.run(
            ["git", "ls-files", "--others", "--ignored",
             "--exclude-standard", "--directory", "-z"],
            cwd=directory,
            capture_output=True,
            text=True,
            check=True
        )

        return {
            item.rstrip("/")
            for item in result.stdout.split("\0")
            if item
        }

    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Warning: Git ignore detection unavailable.")
        return set()


def create_zip(source: Path, output: Path):
    source = source.resolve()
    output = output.resolve()

    if not source.is_dir():
        raise ValueError(f"Source directory does not exist: {source}")

    ignored = get_git_ignored_files(source)

    # Always exclude Git metadata.
    ignored.add(".git")

    file_count = 0
    skipped_count = 0

    with zipfile.ZipFile(
        output, "w", compression=zipfile.ZIP_DEFLATED
    ) as archive:

        for root, dirs, files in os.walk(source):

            root_path = Path(root)
            relative_root = root_path.relative_to(source)

            # Remove ignored directories from traversal.
            dirs[:] = [
                d for d in dirs
                if str((relative_root / d).as_posix()) not in ignored
            ]

            for filename in files:
                file_path = root_path / filename
                relative_path = file_path.relative_to(source)
                archive_path = relative_path.as_posix()

                if archive_path in ignored:
                    skipped_count += 1
                    continue

                # Avoid including the ZIP itself.
                if file_path.resolve() == output:
                    continue

                # Avoid following symlinks outside the repository.
                if file_path.is_symlink():
                    skipped_count += 1
                    continue

                archive.write(file_path, archive_path)
                file_count += 1

    print(f"\nArchive created: {output}")
    print(f"Files included: {file_count}")
    print(f"Files/directories skipped: {skipped_count}")


def main():
    parser = argparse.ArgumentParser(
        description="Zip a directory while respecting .gitignore."
    )

    parser.add_argument(
        "directory",
        type=Path,
        help="Directory to archive"
    )

    parser.add_argument(
        "-o", "--output",
        type=Path,
        help="Output ZIP file (default: directory-name.zip)"
    )

    args = parser.parse_args()

    source = args.directory.expanduser()

    output = (
        args.output.expanduser()
        if args.output
        else Path(f"{source.resolve().name or 'repository'}.zip")
    )

    try:
        create_zip(source, output)
    except (ValueError, OSError, zipfile.BadZipFile) as error:
        print(f"Error: {error}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
