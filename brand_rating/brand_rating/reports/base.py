"Базовая инфраструктура для реализации отчётов."

from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, Mapping, Sequence


@dataclass(frozen=True)
class ReportResult:
    headers: Sequence[str]
    rows: Sequence[Sequence[object]]


class Report:
    name: str

    def generate(self, rows: Iterable[Mapping[str, str]]) -> ReportResult:  # pragma: no cover - interface
        raise NotImplementedError


class ReportRegistry:
    def __init__(self) -> None:
        self._reports: dict[str, Report] = {}

    def register(self, report: Report) -> None:
        self._reports[report.name] = report

    def get(self, name: str) -> Report:
        try:
            return self._reports[name]
        except KeyError as exc:
            raise ValueError(f"Неизвестный отчёт: {name}") from exc

    def available_reports(self) -> Sequence[str]:
        return tuple(sorted(self._reports))
