"Формирование отчёта со средним рейтингом по брендам."

from __future__ import annotations

from collections import defaultdict
from statistics import mean
from typing import Iterable, Mapping

from .base import Report, ReportResult
from . import registry


class AverageRatingReport(Report):
    name = "average-rating"

    def generate(self, rows: Iterable[Mapping[str, str]]) -> ReportResult:
        ratings_by_brand: dict[str, list[float]] = defaultdict(list)
        for row in rows:
            brand = row.get("brand")
            rating = row.get("rating")
            if not brand or rating is None:
                continue
            try:
                rating_value = float(rating)
            except ValueError:
                raise ValueError(f"Не удалось преобразовать рейтинг '{rating}' к числу.")
            ratings_by_brand[brand].append(rating_value)

        report_rows = [
            # Округляем среднее до двух знаков, чтобы вывод смотрелся компактно.
            (brand, round(mean(values), 2))
            for brand, values in ratings_by_brand.items()
            if values
        ]
        report_rows.sort(key=lambda item: (-item[1], item[0]))

        return ReportResult(headers=("бренд", "средний рейтинг"), rows=report_rows)


registry.register(AverageRatingReport())
