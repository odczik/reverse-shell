FROM node
WORKDIR /SC
COPY . /SC
RUN npm install
ENV PORT 8080
EXPOSE 8080
CMD ["npm", "start"]