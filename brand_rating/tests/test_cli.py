"Тесты для командного интерфейса."

from __future__ import annotations

import pytest

from brand_rating.cli import build_parser, main, render, run
from brand_rating.data import read_rows
from brand_rating.reports import registry


def write_csv(path, rows):
    header = "name,brand,price,rating\n"
    content = "".join(
        f"{row['name']},{row['brand']},{row['price']},{row['rating']}\n" for row in rows
    )
    path.write_text(header + content, encoding="utf-8")


def test_run_calculates_average_across_files(tmp_path):
    file1 = tmp_path / "data1.csv"
    file2 = tmp_path / "data2.csv"
    write_csv(
        file1,
        [
            {"name": "iphone 15", "brand": "apple", "price": "999", "rating": "4.9"},
            {"name": "ipad", "brand": "apple", "price": "799", "rating": "4.7"},
        ],
    )
    write_csv(
        file2,
        [
            {"name": "s23", "brand": "samsung", "price": "1199", "rating": "4.8"},
            {"name": "watch", "brand": "apple", "price": "599", "rating": "4.8"},
        ],
    )

    output = run([str(file1), str(file2)], "average-rating")
    assert output.headers == ("бренд", "средний рейтинг")
    assert output.rows[0] == ("apple", pytest.approx(4.8, rel=1e-6))
    assert output.rows[1] == ("samsung", pytest.approx(4.8, rel=1e-6))
    assert output.rows[0][1] >= output.rows[1][1]


def test_read_rows_raises_for_unknown_file(tmp_path):
    missing_file = tmp_path / "missing.csv"
    with pytest.raises(FileNotFoundError):
        read_rows([str(missing_file)])


def test_run_rejects_unknown_report(tmp_path):
    file_path = tmp_path / "data.csv"
    write_csv(
        file_path,
        [
            {"name": "iphone", "brand": "apple", "price": "999", "rating": "4.9"},
        ],
    )
    with pytest.raises(ValueError):
        run([str(file_path)], "unknown")


def test_average_rating_report_validates_rating(tmp_path):
    file_path = tmp_path / "data.csv"
    write_csv(
        file_path,
        [
            {"name": "iphone", "brand": "apple", "price": "999", "rating": "invalid"},
        ],
    )
    with pytest.raises(ValueError):
        run([str(file_path)], "average-rating")


def test_main_prints_table(tmp_path, capsys):
    file_path = tmp_path / "data.csv"
    write_csv(
        file_path,
        [
            {"name": "iphone", "brand": "apple", "price": "999", "rating": "4.9"},
            {"name": "galaxy", "brand": "samsung", "price": "1199", "rating": "4.8"},
        ],
    )

    exit_code = main(["--files", str(file_path), "--report", "average-rating"])
    assert exit_code == 0
    table_output = capsys.readouterr().out.strip()
    assert "бренд" in table_output
    assert "средний рейтинг" in table_output


def test_build_parser_has_known_reports():
    parser = build_parser(registry.available_reports())
    with pytest.raises(SystemExit):
        parser.parse_args([])
    args = parser.parse_args(["--files", "data.csv", "--report", "average-rating"])
    assert args.report == "average-rating"
