FROM agrigorev/zoomcamp-model:2025

WORKDIR /app

# Копируем всё внутрь контейнера
COPY . .

# Устанавливаем зависимости
RUN pip install fastapi uvicorn scikit-learn pydantic

# Добавляем src в PYTHONPATH, чтобы uvicorn видел твой модуль
ENV PYTHONPATH="/app/src"

EXPOSE 9696

# Запускаем predict.py, где app = FastAPI()
CMD ["uvicorn", "predict:app", "--host", "0.0.0.0", "--port", "9696"]
