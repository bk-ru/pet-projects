"CLI-точка входа для формирования отчётов по брендам."

from __future__ import annotations

import argparse
from dataclasses import dataclass
from typing import Iterable, Sequence

from tabulate import tabulate

from .data import read_rows
from .reports import registry


@dataclass(frozen=True)
class ReportOutput:
    headers: Sequence[str]
    rows: Sequence[Sequence[object]]


def build_parser(available_reports: Iterable[str]) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Формирование аналитических отчётов на основе рейтингов товаров."
    )
    parser.add_argument(
        "--files",
        nargs="+",
        required=True,
        metavar="PATH",
        help="Пути к входным CSV-файлам с данными о товарах.",
    )
    parser.add_argument(
        "--report",
        required=True,
        choices=sorted(set(available_reports)),
        help="Тип отчёта, который необходимо сформировать.",
    )
    return parser


def run(files: Sequence[str], report_name: str) -> ReportOutput:
    # Загружаем данные из всех файлов и передаём их выбранному отчёту.
    rows = read_rows(files)
    report = registry.get(report_name)
    return report.generate(rows)


def render(output: ReportOutput) -> str:
    return tabulate(output.rows, headers=output.headers, tablefmt="github")


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_parser(registry.available_reports())
    args = parser.parse_args(argv)
    output = run(args.files, args.report)
    table = render(output)
    print(table)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
