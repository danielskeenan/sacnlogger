import setPageTitle from "../common/setPageTitle.ts";
import {APP_NAME} from "../common/constants.ts";

export function Component() {
    setPageTitle();

    return (
        <>
            <h1 className="display-5 fw-bold">{APP_NAME}</h1>
        </>
    );
}
