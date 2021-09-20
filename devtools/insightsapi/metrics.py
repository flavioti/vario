from datetime import date, datetime
from fastapi import FastAPI
from fastapi import Response

from sqlalchemy import Column, Integer, create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import Session, sessionmaker
from sqlalchemy.sql.functions import now
from sqlalchemy.sql.sqltypes import Float

engine = create_engine(
    "postgresql+psycopg2://postgres:postgres@localhost:5432/postgres"
    "?options=--search_path=public"
)

Base = declarative_base(engine)
meta = Base.metadata

SessionMaker: Session = sessionmaker(
    autocommit=False,
    autoflush=False,
    bind=engine,
)


class Metrics(Base):
    __tablename__ = "metrics"

    id = Column(Integer, primary_key=True)
    dev_ts = Column(Integer, nullable=True, comment="Device epoch (GMT0)")
    rec_ts = Column(Integer, nullable=True, comment="Received epoch (GMT0)")
    bte = Column(Float)
    bal = Column(Float)
    bpr = Column(Float)
    glo = Column(Float)
    gla = Column(Float)
    gal = Column(Float)
    gsa = Column(Integer)
    loo = Column(Integer)
    sbv = Column(Float)
    sbp = Column(Float)


Base.metadata.create_all(engine, checkfirst=True)

session: Session = SessionMaker()

app = FastAPI()


@app.get("/")
def metrics(
    dev_ts=None,
    rec_ts=None,
    bte=None,
    bal=None,
    bpr=None,
    glo=None,
    gla=None,
    gal=None,
    gsa=None,
    loo=None,
    sbv=None,
    sbp=None,
):
    metrics = Metrics()
    metrics.dev_ts = dev_ts
    metrics.rec_ts = datetime.now().timestamp()
    metrics.bte = bte
    metrics.bal = bal
    metrics.bpr = bpr
    metrics.glo = glo
    metrics.gla = gla
    metrics.gal = gal
    metrics.gsa = gsa
    metrics.loo = loo
    metrics.sbv = sbv
    metrics.sbp = sbp

    session.add(metrics)
    session.commit()
    return Response(status_code=204)


# uvicorn metrics:app --host 192.168.99.14
