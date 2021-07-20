import { fetchWithDelay } from './fetch';
const url = 'https://jsonplaceholder.typicode.com/posts';

const fetchPosts = () => fetchWithDelay(url)
  .then((posts) => posts.slice(0, 2));

export const postAPI = {
  fetchPosts,
};