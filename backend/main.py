from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field
from typing import Optional, List
from datetime import datetime
from pymongo import MongoClient
from bson.objectid import ObjectId
import os

MONGO_URI = os.getenv("MONGO_URI", "mongodb://localhost:27017")
DB_NAME = os.getenv("MONGO_DB_NAME", "screenshots_db")
client = MongoClient(MONGO_URI)
db = client[DB_NAME]
collection = db["screenShots"]

app = FastAPI()

class ScreenshotCreate(BaseModel):
    image_url: str
    date_taken: datetime = Field(default_factory=datetime.utcnow)
    description: Optional[str] = None

class Screenshot(ScreenshotCreate):
    id: str


def serialize(doc) -> Screenshot:
    return Screenshot(
        id=str(doc["_id"]),
        image_url=doc["image_url"],
        date_taken=doc["date_taken"],
        description=doc.get("description"),
    )

@app.post("/screenshots", response_model=Screenshot)
def create_screenshot(data: ScreenshotCreate):
    result = collection.insert_one(data.dict())
    doc = collection.find_one({"_id": result.inserted_id})
    if not doc:
        raise HTTPException(status_code=500, detail="Failed to create screenshot")
    return serialize(doc)

@app.get("/screenshots", response_model=List[Screenshot])
def list_screenshots():
    return [serialize(doc) for doc in collection.find()]
