"Функции для чтения входных CSV-файлов."

from __future__ import annotations

import csv
from pathlib import Path
from typing import Iterable, Iterator, Mapping


def _open_csv(path: Path) -> Iterator[Mapping[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        yield from reader


def read_rows(files: Iterable[str]) -> list[Mapping[str, str]]:
    rows: list[Mapping[str, str]] = []
    for file_path in files:
        path = Path(file_path)
        if not path.exists():
            raise FileNotFoundError(f"Файл не найден: {path}")
        # Объединяем все строки в один список, чтобы отчёты видели общие данные.
        rows.extend(_open_csv(path))
    return rows
