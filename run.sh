#!/bin/bash
docker build -t gemini_interview . && docker run -i gemini_interview /app/build/app/matching_engine
