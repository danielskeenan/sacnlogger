import "./App.scss";
import {Container, Nav, Navbar as BsNavbar} from "react-bootstrap";
import {APP_NAME} from "./common/constants.ts";
import {isRouteErrorResponse, Link, Outlet, useNavigation, useRouteError} from "react-router";
import {Loading} from "./common/components/Loading.tsx";
import {Links} from "./routes.ts";
import ConfigTitle from "./config/ConfigTitle.tsx";

export default function App() {
    const navigation = useNavigation();
    const isNavigating = Boolean(navigation.location);

    return (
        <div className="content-wrapper">
            <Navbar/>

            <Container as="main">
                {isNavigating && <Loading/>}
                <Outlet/>
            </Container>
        </div>
    )
}

export function Navbar() {
    return (
        <BsNavbar collapseOnSelect expand="lg"
                  className="msacn-navbar-main"
                  variant="dark"
                  bg="primary"
                  fixed="top"
        >
            <BsNavbar.Brand as={Link} to={Links.FRONT}>
                {APP_NAME}
            </BsNavbar.Brand>
            <BsNavbar.Toggle aria-controls="msacn-navbar-content"/>
            <BsNavbar.Collapse id="msacn-navbar-content">
                <Nav className="me-auto">
                    <Nav.Link as={Link} to={Links.CONFIG}><ConfigTitle/></Nav.Link>
                    <Nav.Link href="/doc" target="_blank">
                        Help
                    </Nav.Link>
                </Nav>
            </BsNavbar.Collapse>
        </BsNavbar>
    );
}

export function ErrorBoundary() {
    const error = useRouteError();
    if (isRouteErrorResponse(error)) {
        return (
            <>
                <h1>
                    {error.status} {error.statusText}
                </h1>
                <p>{error.data}</p>
            </>
        );
    } else if (error instanceof Error) {
        return (
            <div>
                <h1>Error</h1>
                <p>{error.message}</p>
                <p>The stack trace is:</p>
                <pre>{error.stack}</pre>
            </div>
        );
    } else {
        return <h1>Unknown Error</h1>;
    }
}
