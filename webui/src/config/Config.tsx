import "./Config.scss";
import {faMinus, faPlus} from "@fortawesome/free-solid-svg-icons";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {sortedUniq} from "lodash";
import {useCallback, useEffect, useId, useState} from "react";
import {Button, Form, ListGroup, Modal, Tab, Tabs} from "react-bootstrap";
import {isValidUniverse, UNIVERSE_MAX, UNIVERSE_MIN} from "../common/constants.ts";
import setPageTitle from "../common/setPageTitle.ts";
import useFocus from "../common/useFocus.ts";
import useHostHttp from "../common/useHostHttp.ts";
import useOnNumberChange from "../common/useOnNumberInputChange.ts";
import {Config} from "../messages/config.ts";
import ConfigTitle from "./ConfigTitle.tsx";

export function Component() {
    setPageTitle("Config");
    // Properties.
    const [universes, setUniverses] = useState<number[]>([]);
    const [pap, setPap] = useState(false);
    // State.
    const [ready, setReady] = useState(false);
    const [addUniverseVisible, setAddUniverseVisible] = useState(false);
    const showAddUniverse = useCallback(() => {
        setAddUniverseVisible(true)
    }, [setAddUniverseVisible]);
    const hideAddUniverse = useCallback(() => {
        setAddUniverseVisible(false)
    }, [setAddUniverseVisible]);

    // Getters.
    const {hostGet} = useHostHttp();
    useEffect(() => {
        hostGet("/rpc/config")
            .then((r) => {
                const msg = Config.getRootAsConfig(r.data).unpack();
                setUniverses(msg.universes);
                setPap(msg.usePap);
                setReady(true);
            })
    }, [hostGet, setReady, setUniverses, setPap]);

    // Setters.
    const addUniverse = useCallback((newUniverse: number) => {
        let newUniverses = [...universes, newUniverse];
        newUniverses.sort((a, b) => a - b);
        newUniverses = sortedUniq(newUniverses);
        setUniverses(newUniverses);
    }, [universes, setUniverses]);
    const removeUniverse = useCallback((oldUniverse: number) => {
        setUniverses(universes.filter(univ => univ != oldUniverse));
    }, [universes, setUniverses]);
    const togglePap = useCallback(() => {
        setPap(!pap);
    }, [pap, setPap]);

    // Helpers.
    const nextUniverse = useCallback(() => {
        const lastUniverse = universes.at(-1);
        if (lastUniverse === undefined) {
            return undefined;
        }
        return lastUniverse + 1;
    }, [universes]);

    return (
        <>
            <h1><ConfigTitle/></h1>

            {ready && (
                <Tabs>
                    <Tab title="sACN" eventKey="sacn">
                        <Form>
                            {/* Universes */}
                            <Form.Group className="mb-3">
                                <Form.Label>Universes</Form.Label>
                                <ListGroup className="sacnlogger-config-universes">
                                    {universes.length == 0 && (
                                        <ListGroup.Item>No universes</ListGroup.Item>
                                    )}
                                    {universes.map((universe) => (
                                        <ListGroup.Item key={universe}>
                                            <Button variant="outline-danger" onClick={() => removeUniverse(universe)}>
                                                <FontAwesomeIcon icon={faMinus} title="Remove"/>
                                            </Button>
                                            <div>{universe}</div>
                                        </ListGroup.Item>
                                    ))}
                                </ListGroup>
                                <Button variant="outline-success" onClick={showAddUniverse}>
                                    <FontAwesomeIcon icon={faPlus}/>&nbsp;Add
                                </Button>
                            </Form.Group>

                            {/* PAP */}
                            <Form.Group className="mb-3">
                                <Form.Switch label="Use Per-Address-Priority" checked={pap} onChange={togglePap}/>
                            </Form.Group>
                        </Form>
                    </Tab>
                    <Tab title="System" eventKey="system">

                    </Tab>
                </Tabs>
            )}

            {addUniverseVisible && (
                <AddUniverseDialog visible={addUniverseVisible}
                                   defaultValue={nextUniverse()}
                                   onClose={hideAddUniverse}
                                   onSubmit={addUniverse}
                />
            )}
        </>
    );
}

interface AddUniverseDialogProps {
    visible: boolean,
    defaultValue?: number,
    onClose: () => void,
    onSubmit: (newValue: number) => void,
}

function AddUniverseDialog(props: AddUniverseDialogProps) {
    const [value, setValue] = useState<number | undefined>(props.defaultValue);

    const onChange = useOnNumberChange(setValue, isValidUniverse);
    const onSubmit = () => {
        props.onClose();
        props.onSubmit(value as number);
    };

    const titleId = useId();
    const inputId = useId();
    const [inputRef, focusInput] = useFocus();
    useEffect(() => {
        focusInput();
    }, [inputRef, focusInput]);

    return (
        <Modal show={props.visible} onHide={props.onClose}>
            <Modal.Header closeButton><Modal.Title id={titleId}>Add Universe</Modal.Title></Modal.Header>
            <Modal.Body>
                <Form>
                    <Form.Control ref={inputRef}
                                  id={inputId}
                                  type="number"
                                  value={value ?? ""}
                                  min={UNIVERSE_MIN}
                                  max={UNIVERSE_MAX}
                                  aria-labelledby={titleId}
                                  onChange={onChange}
                    />
                </Form>
            </Modal.Body>
            <Modal.Footer>
                <Button variant="secondary" onClick={props.onClose}>
                    Close
                </Button>
                <Button variant="primary" disabled={value === undefined || !isValidUniverse(value)} onClick={onSubmit}>
                    Add
                </Button>
            </Modal.Footer>
        </Modal>
    );
}
