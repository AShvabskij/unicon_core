import { fetchWithDelay } from './fetch';
const url = 'https://jsonplaceholder.typicode.com/users';

const fetchDevices = () => fetchWithDelay(url);

export const devicesAPI = {
    fetchDevices,
};