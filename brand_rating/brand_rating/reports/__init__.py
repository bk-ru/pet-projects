"Реестр отчётов и встроенные реализации."

from __future__ import annotations

from .base import Report, ReportRegistry

registry = ReportRegistry()

# Import built-in reports so they register themselves.
from . import average_rating  # noqa: E402,F401

__all__ = ["Report", "ReportRegistry", "registry"]
