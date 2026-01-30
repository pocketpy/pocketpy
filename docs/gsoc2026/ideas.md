---
icon: light-bulb
order: 0
label: "Project Ideas"
---


## Idea Title

Build a Vibe Coding Agent in Python for Creating Mobile Games

## Project Size

Medium

## Related Skills

- CPython
- Agentic Programming
- Prompt Engineering
- PIXI.JS Framework

## Description

pocketpy is an organization dedicated to creating game development tools in Python language.
Nowadays, vibe coding has become a popular approach for rapid game development, allowing developers to create games quickly and efficiently by leveraging language models and agentic programming techniques.

For Google Summer of Code 2026, we are looking for a student to develop a vibe coding agent that can assist developers in creating mobile games.
This agent is composed of two main components,
backend and frontend.

The backend part should be developed in CPython,
which is composed of the following modules:

+ Virtual Container. The agent needs to create a virtual linux container for each vibe coding project. This module provides management for users' sources and assets inside the container.
+ AI Service Provider. This module is responsible for communicating with AI service providers, such as OpenAI, to generate code and assets based on user prompts.
+ Persistent Memory. This module stores the state of each vibe coding project, including project progress, user preferences, and other relevant information.
+ Agentic Core. This module uses Persistent Memory and AI Service Provider to implement the agentic programming logic, enabling the agent to understand user prompts and generate appropriate code and assets.
+ PIXI.JS Integration. We decide to use [PIXI.JS](https://pixijs.com/) as the default rendering engine for user projects. This is because PIXI.JS is fully source-driven,
which makes it easier for the agent to generate and modify game code.

The frontend part is optional. Knowing this could help students better understand the whole project.
We aims to create a mobile app using Flutter framework. This app invodes backend services via RESTful APIs,
and provides a user-friendly interface for users to control and run their vibe coding projects.

For more details, we will discuss with the selected student during the community bonding period.
