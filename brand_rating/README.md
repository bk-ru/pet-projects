# Brand Rating Reports

Small command line tool that builds analytical reports from CSV exports with product information.

## Quickstart
- Install dependencies: `pip install -r requirements.txt`
- Show help: `python -m brand_rating --help`
- Generate the average rating report: `python -m brand_rating --files data/products1.csv data/products2.csv --report average-rating`
- Отчёт выводится в консоль на русском языке.

## Architecture Notes
- Reports are implemented as classes in `brand_rating/reports/` and registered in the shared `registry`.
- To add a new report, create a module that subclasses `Report`, implement `generate`, and call `registry.register(...)`.

## Development
- Install dev dependencies: `pip install -r requirements-dev.txt`
- Run tests: `pytest`
