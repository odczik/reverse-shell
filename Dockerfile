FROM node
WORKDIR /SC
COPY . /
ENV PORT 8080
EXPOSE 8080
CMD ["node", "server.js"]