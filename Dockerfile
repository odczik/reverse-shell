FROM node
WORKDIR /SC
COPY . /
RUN npm install ws
ENV PORT 8080
EXPOSE 8080
CMD ["node", "server.js"]
