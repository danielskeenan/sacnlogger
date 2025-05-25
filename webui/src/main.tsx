import {StrictMode} from 'react'
import {createRoot} from 'react-dom/client'
import App, {ErrorBoundary} from './App.tsx'
import {createBrowserRouter, RouterProvider} from "react-router";
import {Links} from "./routes.ts";
import {Loading} from "./common/components/Loading.tsx";

const router = createBrowserRouter([
    {
        path: Links.FRONT,
        Component: App,
        ErrorBoundary: ErrorBoundary,
        HydrateFallback: Loading,
        children: [
            {
                index: true,
                lazy: () => import('./front/Front.tsx'),
            },
        ],
    },
]);

createRoot(document.getElementById('root')!).render(
    <StrictMode>
        <RouterProvider router={router}/>
    </StrictMode>,
)
