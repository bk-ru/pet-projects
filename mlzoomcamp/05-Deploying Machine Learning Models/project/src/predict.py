import pickle
from fastapi import FastAPI
from pydantic import BaseModel

# Загружаем модель
model_file = "src/pipeline_v1.bin"

with open(model_file, "rb") as f_in:
    dv, model = pickle.load(f_in)

# Определяем схему входных данных
class Lead(BaseModel):
    lead_source: str
    number_of_courses_viewed: int
    annual_income: float

app = FastAPI(title="Lead Conversion Prediction API")

@app.post("/predict")
def predict(lead: Lead):
    lead_dict = lead.dict()

    X = dv.transform([lead_dict])
    y_pred = model.predict_proba(X)[0, 1]

    return {"conversion_probability": float(y_pred)}
